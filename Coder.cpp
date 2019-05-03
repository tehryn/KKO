/**
 * File: Coder.cpp
 * Author: Jiri Matejka (xmatej52)
 * Modified: 02. 05. 2019
 * Description: Implementation of Coder class where huffman encoding/decoding is implemented.
 */
#include "Coder.hpp"

void Coder::getFileHeaders( std::vector<uint8_t> & lengths, std::vector<uint8_t> & tree, huffmanCode & coder ) {
    // byte that will be pushed into output
    uint8_t byteToWrite = 0;
    size_t index = this->idx;

    // write len of each code
    for (size_t byte = 0; byte < 256; byte++) {
        uint8_t len = 0;
        if ( coder.find( byte ) != coder.end() ) {
            len = coder[ byte ].size();
            // encode char into bits
            for ( bool bit : coder[ byte ] ) {
                // 8 bits were written -> lets write it into file
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
    // byte that will be pushed into output
    uint8_t byteToWrite = this->byteNotWritten;
    size_t index = this->idx;
    // encodes al of data
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
    // lengths of each encoded bytes
    std::vector<uint8_t> lengths;

    // ecoded data
    std::vector<uint8_t> encoded;

    // bit representation of tree
    std::vector<uint8_t> tree;

    // prealloc memory
    lengths.reserve( 257 );
    lengths.push_back(0);
    tree.reserve( 65536 );
    encoded.reserve( data.size() );

    // set headers
    this->getFileHeaders( lengths, tree, coder );

    // encode data
    this->encode( data, coder, encoded );

    // how many bits has to be ignored
    uint8_t extra = 8 - this->idx;

    // write last byte of encoded data
    if ( extra > 0 ) {
        this->byteNotWritten <<= extra;
    }
    encoded.push_back( this->byteNotWritten );

    // set bit that specifie use of model
    if ( this->useModel ) {
        extra |= ( 1 << 4 );
    }

    // store number of extra bits and use of model into start of file
    lengths[0] = extra;
    this->idx = 0;

    // write lengths
    fwrite( lengths.data(), 1, lengths.size(), outfile );

    // write tree
    fwrite( tree.data(), 1, tree.size(), outfile );

    // write encoded data
    fwrite( encoded.data(), 1, encoded.size(), outfile );
}

void Coder::adaptiveEncode( std::vector<uint8_t> & data, Tree * root, FILE * outfile ) {
    // encoded content
    std::vector<uint8_t> encoded;

    // all nodes of tree
    std::vector<Tree *> nodes;

    // prealloc memory
    encoded.reserve( data.size() );
    nodes.reserve(1024);

    // set depths of nodes
    root->setDepth(0);

    // get reverse inorder of tree (most right node will be at index 0)
    root->reverseInorder( nodes );

    // generates huffman coder
    huffmanCode coder = root->generateHuffmanCode();

    Tree * node = nullptr;
    uint8_t byteToWrite = this->byteNotWritten;
    size_t index = this->idx;

    // when tree will be updated
    size_t change = 1;

    // how many bytes have we parsed yet
    size_t parsed = 0;

    // encode data
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
        // update tree
        if ( parsed == change ) {
            if ( node->update( nodes ) ) {
                root->setDepth( root->depth );
                nodes.clear();
                root->reverseInorder( nodes );
                root->generateHuffmanCode( coder );
            }
            // set new milenium for tree update
            change = Coder::nextUpdate( change, this->effort );
        }
    }

    // how many bits has to be appended
    uint8_t extra = 8 - index;
    if ( extra > 0 ) {
        byteToWrite <<= extra;
    }

    this->idx = 0;

    // set adaptive method into header of file
    extra |= 0b00001000;

    // set use of model into header
    if ( this->useModel ) {
        extra |= 0b00010000;
    }
    // set used effort into header
    extra |= ( this->effort << 5 );

    // write last byte
    encoded.push_back( byteToWrite );

    // write header
    fwrite( &extra, 1, 1, outfile );

    // write encoded data
    fwrite( encoded.data(), 1, encoded.size(), outfile );
}

bool Coder::adaptiveDecode( std::vector<uint8_t> & data, Tree * root, FILE * outfile, uint8_t extra, bool model, int effort ) {
    // decoded bytes
    std::vector<uint8_t> decoded;

    // pointer to encoded data
    uint8_t * dataPtr = data.data();

    // all nodes of tree
    std::vector<Tree *> nodes;

    // memory prealloc
    decoded.reserve( data.size() << 1 );
    nodes.reserve(1024);

    // set depth of tree
    root->setDepth(0);

    // reverse inorder of tree nodes -- most right is first
    root->reverseInorder( nodes );

    // current node in tree (need for decoding)
    Tree * tmp    = root;

    // current byte
    uint8_t byte  = 0;

    // end of loop
    size_t end    = data.size() - ( extra > 0 ? 1 : 0 );

    // starting index
    size_t index  = this->idx;

    // when nect tree update happens
    size_t change = 1;

    // how many bytes were parsed
    size_t parsed = 0;

    // decoding
    while( index < end ) {
        // set new byte
        byte = dataPtr[ index++ ];
        // parse each bit of byte
        for ( uint8_t i = 0; i < 8; i++ ) {
            // test if leaf
            if ( tmp->left == nullptr ) {
                decoded.push_back( tmp->data );
                parsed++;
                // update node
                if ( parsed == change ) {
                    if ( tmp->update( nodes ) ) {
                        root->setDepth( root->depth );
                        nodes.clear();
                        root->reverseInorder( nodes );
                    }
                    change = Coder::nextUpdate( change, effort );
                }
                // reset current node to root
                tmp = root;
            }
            // move to right descendant or left descendant
            tmp = ( byte & ( 1 << ( 7 - i ) ) ) ? tmp->right : tmp->left;
        }
    }

    // some bits have to be ignored
    if ( extra > 0 ) {
        extra = 8 - extra;

        // loads new byte
        byte = dataPtr[ index++ ];

        // read not all of bits in file
        for ( size_t i = 0; i < extra; i++ ) {
            // test if leaf
            if ( tmp->left == nullptr ) {
                decoded.push_back( tmp->data );
                parsed++;
                // updates tree
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

    // write last byte into output
    if ( tmp->left == nullptr ) {
        decoded.push_back( tmp->data );
        tmp = root;
    }

    // test if decde failed
    if ( tmp != root ) {
        return false;
    }

    this->idx = 0;

    // parse data with model if model was used
    if ( model ) {
        Coder::differences2data( decoded );
    }

    // writes output to file
    fwrite( decoded.data(), 1, decoded.size(), outfile );
    return true;
}

bool Coder::staticDecode( std::vector<uint8_t> & data, FILE * outfile, uint8_t extra, bool model ) {
    // test minimum file size
    if ( data.size() <= 257 ) {
        return false;
    }

    // expected size of file -- ned to test it
    size_t len = 0;

    // decoded data
    std::vector<uint8_t> output;

    // lengths of characters in tree
    std::vector<uint8_t> lengths;

    // memory prealloc
    output.reserve( data.size() >> 1 );
    lengths.reserve( 256 );

    // coder needed to build tree
    huffmanCode coder;

    // current byte
    uint8_t byte = 0;

    // pointer to encoded data
    uint8_t * dataPtr = data.data();

    // current index
    size_t index = this->idx;

    // load lengths from header of file
    while( index < 257 ) {
        byte = dataPtr[ index++ ];
        lengths.push_back( byte );
        len += byte;
    }

    // test if filesize is still ok
    uint8_t extraRead = len % 8;
    if ( data.size() <= 257 + (len + 8 - extraRead ) / 8 ) {
        return false;
    }

    // sets used coder for encoding
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

    // build tree from coder
    Tree * root = new Tree( coder );

    // current node in tree
    Tree * tmp = root;

    // read rest of byte (if there is)
    while ( shift < 8 ) {
        if ( tmp->right == nullptr && tmp->left == nullptr ) {
            output.push_back( tmp->data );
            tmp = root;
        }
        tmp = ( byte & ( 1 << ( 7 - shift++ ) ) ) ? tmp->right : tmp->left;
    }

    // when loop ends
    size_t end = data.size() - ( extra > 0 ? 1 : 0 );

    // decode data
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

    // decode last bits, if there are any
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

    // write last byte into output
    if ( tmp->right == nullptr && tmp->left == nullptr ) {
        output.push_back( tmp->data );
        tmp = root;
    }

    // test if data were corrupted
    if ( tmp != root ) {
        return false;
    }

    this->idx = 0;

    delete root;

    // process data with model if model was used during encoding
    if ( model ) {
        Coder::differences2data( output );
    }

    // writes data to output
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

void Coder::huffmanEncode( std::vector<uint8_t> & data, FILE * output ) {
    this->init();
    // use model
    if ( this->useModel ) {
        Coder::data2differences( data );
    }
    // adaptive/static
    if ( this->adaptive ) {
        // init adaptive tree
        Tree * tree = Tree::initAdaptiveTree();
        this->adaptiveEncode( data, tree, output );
        delete tree;
    }
    else {
        // loal leaves
        std::vector<Tree *> leaves = Tree::loadLeafNodes( data );
        // build tree
        Tree * tree = Tree::buildTree( leaves );
        // creates coder
        huffmanCode coder = tree->generateHuffmanCode();
        // tree is not needed anymore
        delete tree;
        this->staticEncode( data, coder, output );
    }
}

bool Coder::huffmanDecode( std::vector<uint8_t> & data, FILE * output ) {
    this->init();
    // test minimum filesize
    if ( data.size() == 0 ) {
        return false;
    }
    // set index to 1
    this->idx = 1;
    // loads settings from fisrt byte
    uint8_t settings = data[0];
    uint8_t extra  = 0b00000111 & settings;
    bool adaptive  = 0b00001000 & settings;
    bool model     = 0b00010000 & settings;
    int  effort    = (0b11100000 & settings) >> 5;
    // adaptive/static
    if ( adaptive ) {
        // init tree
        Tree * tree = Tree::initAdaptiveTree();
        bool ret = this->adaptiveDecode( data, Tree::initAdaptiveTree(), output, extra, model, effort );
        delete tree;
        return ret;
    }
    else {
        return this->staticDecode( data, output, extra, model );
    }
}