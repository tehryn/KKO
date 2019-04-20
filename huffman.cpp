#include "huffman.hpp"
#include "Coder.hpp"

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
                    std::cerr << "Unable to read data from input file." << std::endl;
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
                else if ( std::string( "adaptive" ) == std::string( optarg ) ) {
                    coding = ADAPTIVE;
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

    Coder huffmanImp = Coder( useModel, coding == ADAPTIVE );

    if ( mode == COMPRESS ) {
        huffmanImp.huffmanEncode( inputData, outfile );
    }
    else {
        if ( !huffmanImp.huffmanDecode( inputData, outfile ) ) {
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

void printMap( huffmanCode & code ) {
    for ( huffmanCode::const_iterator it = code.begin(); it != code.end(); it++ ) {
    std::cout << +it->first << " ";
    std::copy(it->second.begin(), it->second.end(),
    std::ostream_iterator<bool>(std::cout));
    std::cout << std::endl;
    }
}