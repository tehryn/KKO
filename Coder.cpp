#include "Coder.hpp"
#include "huffman.hpp" // TODO
void Coder::getFileHeaders( std::vector<uint8_t> & lengths, std::vector<uint8_t> & tree, huffmanCode & coder ) {
    uint8_t byteToWrite = 0;
    for (size_t byte = 0; byte < 256; byte++) {
        uint8_t len = 0;
        if ( coder.find( byte ) != coder.end() ) {
            len = coder[ byte ].size();
            for ( bool bit : coder[ byte ] ) {
                if ( this->idx++ >= 8 ) {
                    tree.push_back( byteToWrite );
                    byteToWrite = 0;
                    this->idx = 1;
                }
                byteToWrite = (byteToWrite << 1) | bit;
            }
        }
        lengths.push_back( len );
    }
    this->byteNotWritten = byteToWrite;
}

void Coder::encode( std::vector<uint8_t> & data, huffmanCode & coder, std::vector<uint8_t> & encoded ) {
    uint8_t byteToWrite = this->byteNotWritten;
    for ( uint8_t byte : data ) {
        for( bool bit : coder[ byte ] ) {
            if ( this->idx >= 8 ) {
                encoded.push_back( byteToWrite );
                byteToWrite = 0;
                this->idx = 0;
            }
            byteToWrite = (byteToWrite << 1) | bit;
            this->idx++;
        }
    }
    this->byteNotWritten = byteToWrite;
}

void Coder::staticEncode( std::vector<uint8_t> & data, huffmanCode & coder, FILE * outfile ) {
    std::vector<uint8_t> lengths;
    std::vector<uint8_t> encoded;
    std::vector<uint8_t> tree;
    lengths.reserve( 257 );
    tree.reserve( 65536 );
    encoded.reserve( data.size() );

    this->getFileHeaders( lengths, tree, coder );
    this->encode( data, coder, encoded );
    uint8_t byteToWrite = this->byteNotWritten;
    uint8_t extra = 8 - this->idx;
    if ( extra > 0 ) {
        byteToWrite <<= extra;
    }
    encoded.push_back( byteToWrite );
    //DEBUG_LINE( "last: ", +byteToWrite );
    //DEBUG_LINE( "extra: ", +extra );
    //printMap( coder );
    lengths.push_back( extra );

    fwrite( lengths.data(), 1, lengths.size(), outfile );
    fwrite( tree.data(), 1, tree.size(), outfile );
    fwrite( encoded.data(), 1, encoded.size(), outfile );
}



bool Coder::staticDecode( std::vector<uint8_t> & data, FILE * outfile ) {
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

    // nacteme delky
    while( this->idx < 256 ) {
        byte = dataPtr[ this->idx++ ];
        lengths.push_back( byte );
        len += byte;
        //std::cout << +byte << '\n';
    }

    // nacteme extra bity ktere musime pak ignorovat
    uint8_t extra = dataPtr[ this->idx++ ];
    //DEBUG_LINE( "extra: ", +extra );
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
                byte = dataPtr[ this->idx++ ];
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
    while( this->idx < end ) {
        byte = dataPtr[ this->idx++ ];
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
        byte = dataPtr[ this->idx++ ];
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
    //DEBUG_LINE( "last: ", +byte );

    if ( tmp != root ) {
        return false;
    }
    delete root;
    fwrite( output.data(), 1, output.size(), outfile );
    return true;
}