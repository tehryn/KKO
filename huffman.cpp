#include "huffman.hpp"

void printMap( huffmanCode & code ) {
    for ( huffmanCode::const_iterator it = code.begin(); it != code.end(); it++ ) {
    std::cout << +it->first << " ";
    std::copy(it->second.begin(), it->second.end(),
    std::ostream_iterator<bool>(std::cout));
    std::cout << std::endl;
    }
}

void compressData( std::vector<uint8_t> data, huffmanCode & coder, FILE * outfile ) {
    std::vector<uint8_t> lengths;
    std::vector<uint8_t> encoded;
    std::vector<uint8_t> tree;
    lengths.reserve(257);
    tree.reserve( 65536 );
    encoded.reserve( data.size() );

    uint8_t idx = 0;
    uint8_t byteToWrite = 0;
    for (size_t byte = 0; byte < 256; byte++) {
        uint8_t len = 0;
        if ( coder.find( byte ) != coder.end() ) {
            len = coder[ byte ].size();
            for ( bool bit : coder[ byte ] ) {
                if ( idx++ >= 8 ) {
                    tree.push_back( byteToWrite );
                    byteToWrite = 0;
                    idx = 1;
                }
                byteToWrite = (byteToWrite << 1) | bit;
            }
        }
        // std::cout << +len << '\n';
        lengths.push_back( len );
    }

    for ( uint8_t byte : data ) {
        for( bool bit : coder[ byte ] ) {
            if ( idx >= 8 ) {
                encoded.push_back( byteToWrite );
                byteToWrite = 0;
                idx = 0;
            }
            byteToWrite = (byteToWrite << 1) | bit;
            idx++;
        }
    }

    uint8_t extra = 8 - idx;
    if ( extra > 0 ) {
        byteToWrite <<= extra;
    }
    encoded.push_back( byteToWrite );

    lengths.push_back( extra );
    std::cerr << +byteToWrite << std::endl;
    //std::cerr << tree.size() << '\n';
    std::cerr << +extra << '\n';
    printMap( coder );
    fwrite( lengths.data(), 1, lengths.size(), outfile );
    fwrite( tree.data(), 1, tree.size(), outfile );
    fwrite( encoded.data(), 1, encoded.size(), outfile );
}

bool decompressData( std::vector<uint8_t> data, FILE * outfile ) {
    if ( data.size() <= 257 ) {
        return false;
    }
    size_t idx = 0;
    size_t len = 0;
    std::vector<uint8_t> output;
    output.reserve( data.size() >> 1 );
    std::vector<uint8_t> lengths;
    lengths.reserve( 256 );
    huffmanCode coder;
    uint8_t byte = 0;
    uint8_t * dataPtr = data.data();

    // nacteme delky
    while( idx < 256 ) {
        byte = dataPtr[ idx++ ];
        lengths.push_back( byte );
        len += byte;
        //std::cout << +byte << '\n';
    }

    // nacteme extra bity ktere musime pak ignorovat
    uint8_t extra = dataPtr[ idx++ ];

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
                byte = dataPtr[ idx++ ];
                shift = 0;
            }
            code.push_back( byte & ( 1 << ( 7 - shift++ ) ) );
            bit++;
        }
        coder[ key++ ] = code;
        bit = 0;
        code.clear();
    }

    Tree * root = new Tree( coder );
    Tree * tmp = root;

    while ( shift < 8 ) {
        if ( tmp->right == nullptr && tmp->left == nullptr ) {
            output.push_back( tmp->data );
            //std::cout << +tmp->data << '\n';
            tmp = root;
        }
        tmp = ( byte & ( 1 << ( 7 - shift++ ) ) ) ? tmp->right : tmp->left;
    }

    size_t end = data.size() - ( extra > 0 ? 1 : 0 );
    while( idx < end ) {
        byte = dataPtr[ idx++ ];
        for ( uint8_t i = 0; i < 8; i++ ) {
            if ( tmp->right == nullptr && tmp->left == nullptr ) {
                output.push_back( tmp->data );
                //std::cout << +tmp->data << '\n';
                tmp = root;
            }
            tmp = ( byte & ( 1 << ( 7 - i ) ) ) ? tmp->right : tmp->left;
        }
    }

    if ( extra > 0 ) {
        extra = 8 - extra;
        byte = dataPtr[ idx++ ];
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
    std::cerr << +byte  << std::endl;
    std::cerr << +extra << std::endl;
    printMap( coder );

    if ( tmp != root ) {
        return false;
    }

    fwrite( output.data(), 1, output.size(), outfile );
    return true;
}

int main( int argc, char **argv ) {
    int opt, mode = UNSET, coding = UNSET;
    std::vector<uint8_t> inputData;
    bool useModel = false;

    FILE * outfile = nullptr;
    while ( ( opt = getopt( argc, argv, ":cdh:mi:o:" ) ) != -1 ) {
        switch ( opt ) {
            case 'c':
                if ( mode != UNSET ) {
                    std::cerr << "Unable to combine parameters -c and -d" << std::endl;
                    return INVALID_ARGUMENTS;
                }
                mode = COMPRESS;
                break;
            case 'd':
                if ( mode != UNSET ) {
                    std::cerr << "Unable to combine parameters -c and -d" << std::endl;
                    return INVALID_ARGUMENTS;
                }
                mode = DECOMPRESS;
                break;
            case 'm':
                if ( useModel ) {
                    std::cerr << "Parameter -m is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                useModel = true;
            case 'i':
                if ( inputData.size() != 0 ) {
                    std::cerr << "Parameter -i is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                inputData = readBytes( optarg );
                if ( inputData.size() == 0 ) {
                    std::cerr << "Unable to read data from file." << std::endl;
                    return FILE_ERROR;
                }
                break;
            case 'o':
                if ( outfile != nullptr ) {
                    std::cerr << "Parameter -o is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                outfile = fopen( optarg, "w" );
                if ( outfile == nullptr ) {
                    std::cerr << "Unable to open output file" << std::endl;
                    return FILE_ERROR;
                }
                break;
            case 'h':
                if ( coding != UNSET ) {
                    std::cerr << "Parameter -h is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                if ( std::string( "static" ) == std::string( optarg ) ) {
                    coding = STATIC;
                }
                else if ( std::string( "dynamic" ) == std::string( optarg ) ) {
                    coding = DYNAMIC;
                }
                else {
                    std::cerr << "Parameter unknown argument of -h option." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                break;
            case ':':
                if ( optopt == 'h' ) {
                    print_help();
                    return 0;
                }
                std::cerr << "Option "<< static_cast<char>(optopt) << " requires an argument." << std::endl;
                return INVALID_ARGUMENTS;
            case '?':
                std::cerr << "unknown option: "<< optopt << std::endl;
                break;
        }
    }

    if ( coding == UNSET || mode == UNSET || outfile == nullptr || inputData.size() == 0 ) {
        std::cerr << "Not all program arguments were specified. Run './huffman -h' for help." << std::endl;
        if ( outfile != nullptr ) {
            fclose( outfile );
        }
        return INVALID_ARGUMENTS;
    }

    if ( mode == COMPRESS ) {
        std::vector<Tree *> leafs = Tree::loadLeafNodes( inputData );


        Tree * tree = Tree::buildTree( leafs );
        huffmanCode coder = tree->generateHuffmanCode();
        //std::cout << *tree << '\n';
        //delete tree;


        compressData( inputData, coder, outfile );
    }
    else {
        if ( !decompressData( inputData, outfile ) ) {
            std::cerr << "Corrupted input file." << std::endl;
            return CORRUPTED_INPUT;
        }
    }

    fclose( outfile );
    return 0;
}

void print_help() {
    std::cout << "./huffman -c|d -h static|adaptive [-m] -i INPUT -o OUTPUT" << std::endl;
}

std::vector<uint8_t> readBytes( std::string filename ) {
    std::ifstream file( filename, std::ios::binary );
    return std::vector<uint8_t>( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );
}
