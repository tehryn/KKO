#include "Coder.hpp"

void Coder::getFileHeaders( std::vector<uint8_t> & lengths, std::vector<uint8_t> & tree, huffmanCode & coder ) {
    uint8_t byteToWrite = 0;
    size_t index = this->idx;
    for (size_t byte = 0; byte < 256; byte++) {
        uint8_t len = 0;
        if ( coder.find( byte ) != coder.end() ) {
            len = coder[ byte ].size();
            for ( bool bit : coder[ byte ] ) {
                if ( index++ >= 8 ) {
                    tree.push_back( byteToWrite );
                    byteToWrite = 0;
                    index = 1;
                }
                byteToWrite = (byteToWrite << 1) | bit;
            }
        }
        lengths.push_back( len );
    }
    this->idx = index;
    this->byteNotWritten = byteToWrite;
}

void Coder::encode( std::vector<uint8_t> & data, huffmanCode & coder, std::vector<uint8_t> & encoded ) {
    uint8_t byteToWrite = this->byteNotWritten;
    size_t index = this->idx;
    for ( uint8_t byte : data ) {
        for( bool bit : coder[ byte ] ) {
            if ( index >= 8 ) {
                encoded.push_back( byteToWrite );
                byteToWrite = 0;
                index = 0;
            }
            byteToWrite = (byteToWrite << 1) | bit;
            index++;
        }
    }
    this->idx = index;
    this->byteNotWritten = byteToWrite;
}

void Coder::staticEncode( std::vector<uint8_t> & data, huffmanCode & coder, FILE * outfile ) {
    std::vector<uint8_t> lengths;
    std::vector<uint8_t> encoded;
    std::vector<uint8_t> tree;
    lengths.reserve( 257 );
    lengths.push_back(0);
    tree.reserve( 65536 );
    encoded.reserve( data.size() );

    this->getFileHeaders( lengths, tree, coder );
    this->encode( data, coder, encoded );
    uint8_t extra = 8 - this->idx;
    if ( extra > 0 ) {
        this->byteNotWritten <<= extra;
    }
    encoded.push_back( this->byteNotWritten );
    if ( this->useModel ) {
        extra |= ( 1 << 4 );
    }

    lengths[0] = extra;
    this->idx = 0;
    fwrite( lengths.data(), 1, lengths.size(), outfile );
    fwrite( tree.data(), 1, tree.size(), outfile );
    fwrite( encoded.data(), 1, encoded.size(), outfile );
}

void Coder::adaptiveEncode( std::vector<uint8_t> & data, Tree * root, FILE * outfile ) {
    std::vector<uint8_t> encoded;
    encoded.reserve( data.size() );

    std::vector<Tree *> nodes;
    std::vector<Tree *> leaves;
    std::vector<Tree *> rest;
    leaves.reserve(1024);
    nodes.reserve(1024);
    rest.reserve(512);

    root->getLeaves( leaves, nodes );
    root->setDepth(0);
    root->reverseInorder( nodes );

    huffmanCode coder = root->generateHuffmanCode();
    Tree * node = nullptr;
    uint8_t byteToWrite = this->byteNotWritten;
    size_t index = this->idx;
    size_t change = 1;
    size_t parsed = 0;
    for ( uint8_t byte : data ) {
        for( bool bit : coder[ byte ] ) {
            if ( index >= 8 ) {
                encoded.push_back( byteToWrite );
                byteToWrite = 0;
                index = 0;
            }
            byteToWrite = (byteToWrite << 1) | bit;
            index++;
        }
        node = root->find( coder[ byte ] );
        parsed++;
        if ( parsed == change ) {
            if ( node->update( nodes ) ) {
                root->setDepth( root->depth );
                nodes.clear();
                root->reverseInorder( nodes );
                root->generateHuffmanCode( coder );
            }
            change = Coder::nextUpdate( change, this->effort );
        }
    }

    uint8_t extra = 8 - index;
    if ( extra > 0 ) {
        byteToWrite <<= extra;
    }
    this->idx = 0;
    extra |= 0b00001000;
    if ( this->useModel ) {
        extra |= 0b00010000;
    }
    extra |= ( this->effort << 5 );
    encoded.push_back( byteToWrite );

    fwrite( &extra, 1, 1, outfile );
    fwrite( encoded.data(), 1, encoded.size(), outfile );
}

bool Coder::adaptiveDecode( std::vector<uint8_t> & data, Tree * root, FILE * outfile, uint8_t extra, bool model, int effort ) {
    std::vector<uint8_t> decoded;
    decoded.reserve( data.size() << 1 );
    uint8_t * dataPtr = data.data();

    std::vector<Tree *> nodes;
    std::vector<Tree *> leaves;
    std::vector<Tree *> rest;
    leaves.reserve(1024);
    nodes.reserve(1024);
    rest.reserve(512);

    root->getLeaves( leaves, nodes );
    root->setDepth(0);
    root->reverseInorder( nodes );

    Tree * tmp    = root;
    uint8_t byte  = 0;
    size_t end    = data.size() - ( extra > 0 ? 1 : 0 );
    size_t index  = this->idx;
    size_t change = 1;
    size_t parsed = 0;
    while( index < end ) {
        byte = dataPtr[ index++ ];
        for ( uint8_t i = 0; i < 8; i++ ) {
            if ( tmp->left == nullptr ) {
                decoded.push_back( tmp->data );
                parsed++;
                if ( parsed == change ) {
                    if ( tmp->update( nodes ) ) {
                        root->setDepth( root->depth );
                        nodes.clear();
                        root->reverseInorder( nodes );
                    }
                    change = Coder::nextUpdate( change, effort );
                }
                tmp = root;
            }
            tmp = ( byte & ( 1 << ( 7 - i ) ) ) ? tmp->right : tmp->left;
        }
    }
    if ( extra > 0 ) {
        extra = 8 - extra;
        byte = dataPtr[ index++ ];
        for ( size_t i = 0; i < extra; i++ ) {
            if ( tmp->left == nullptr ) {
                decoded.push_back( tmp->data );
                parsed++;
                if ( parsed == change ) {
                    if ( tmp->update( nodes ) ) {
                        root->setDepth( root->depth );
                        nodes.clear();
                        root->reverseInorder( nodes );
                    }
                    change = Coder::nextUpdate( change, effort );
                }
                tmp = root;
            }
            tmp = ( byte & ( 1 << ( 7 - i ) ) ) ? tmp->right : tmp->left;
        }
    }

    if ( tmp->left == nullptr ) {
        decoded.push_back( tmp->data );
        tmp = root;
    }

    if ( tmp != root ) {
        return false;
    }

    this->idx = 0;
    if ( model ) {
        Coder::differences2data( decoded );
    }
    fwrite( decoded.data(), 1, decoded.size(), outfile );
    return true;
}

bool Coder::staticDecode( std::vector<uint8_t> & data, FILE * outfile, uint8_t extra, bool model ) {
    if ( data.size() <= 257 ) {
        return false;
    }
    size_t len = 0;
    std::vector<uint8_t> output;
    output.reserve( data.size() >> 1 );
    std::vector<uint8_t> lengths;
    lengths.reserve( 256 );
    huffmanCode coder;
    uint8_t byte = 0;
    uint8_t * dataPtr = data.data();
    size_t index = this->idx;

    // nacteme delky
    while( index < 257 ) {
        byte = dataPtr[ index++ ];
        lengths.push_back( byte );
        len += byte;
        //std::cout << +byte << '\n';
    }

    uint8_t extraRead = len % 8;
    if ( data.size() <= 257 + (len + 8 - extraRead ) / 8 ) {
        return false;
    }

    // nastavi decoder
    bitSet code;
    uint8_t shift = 8;
    uint8_t key = 0;
    for( uint8_t codeLen : lengths ) {
        uint8_t bit = 0;
        while ( bit < codeLen ) {
            if ( shift >= 8 ) {
                byte = dataPtr[ index++ ];
                shift = 0;
            }
            code.push_back( byte & ( 1 << ( 7 - shift++ ) ) );
            bit++;
        }
        coder[ key++ ] = code;
        bit = 0;
        code.clear();
    }
    //printMap(coder);

    Tree * root = new Tree( coder );
    Tree * tmp = root;

    while ( shift < 8 ) {
        if ( tmp->right == nullptr && tmp->left == nullptr ) {
            output.push_back( tmp->data );
            tmp = root;
        }
        tmp = ( byte & ( 1 << ( 7 - shift++ ) ) ) ? tmp->right : tmp->left;
    }

    size_t end = data.size() - ( extra > 0 ? 1 : 0 );
    while( index < end ) {
        byte = dataPtr[ index++ ];
        for ( uint8_t i = 0; i < 8; i++ ) {
            if ( tmp->right == nullptr && tmp->left == nullptr ) {
                output.push_back( tmp->data );
                tmp = root;
            }
            tmp = ( byte & ( 1 << ( 7 - i ) ) ) ? tmp->right : tmp->left;
        }
    }

    if ( extra > 0 ) {
        extra = 8 - extra;
        byte = dataPtr[ index++ ];
        for ( size_t i = 0; i < extra; i++ ) {
            if ( tmp->right == nullptr && tmp->left == nullptr ) {
                output.push_back( tmp->data );
                tmp = root;
            }
            tmp = ( byte & ( 1 << ( 7 - i ) ) ) ? tmp->right : tmp->left;
        }
    }

    if ( tmp->right == nullptr && tmp->left == nullptr ) {
        output.push_back( tmp->data );
        tmp = root;
    }

    if ( tmp != root ) {
        return false;
    }
    this->idx = 0;
    delete root;
    if ( model ) {
        Coder::differences2data( output );
    }
    fwrite( output.data(), 1, output.size(), outfile );
    return true;
}

void Coder::data2differences( std::vector<uint8_t> & data ) {
    uint8_t prev = 0;
    uint8_t diff = 0;
    for( std::vector<uint8_t>::iterator i = data.begin(); i != data.end(); i++ ) {
        diff = prev - *i;
        prev = *i;
        *i = diff;
    }
}

void Coder::differences2data( std::vector<uint8_t> & data ) {
    uint8_t prev = 0;
    for( std::vector<uint8_t>::iterator i = data.begin(); i != data.end(); i++ ) {
        *i = prev = prev - *i;
    }
}

size_t Coder::nextUpdate( size_t current, int effort ) {
    switch ( effort ) {
        case EFFORT_9: return current + 1;
        case EFFORT_8: return current > 1000 ? current + 10   : current + 1;
        case EFFORT_7: return current > 1000 ? current + 25   : current + 1;
        case EFFORT_6: return current > 1000 ? current + 50   : current + 1;
        case EFFORT_5: return current > 1000 ? current + 100  : current + 1;
        case EFFORT_4: return current > 1000 ? current + 500  : current + 1;
        case EFFORT_3: return current > 1000 ? current + 1000 : current + 1;
        default: break;
    };
    return current << 1;
}
