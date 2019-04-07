#include "huffman.hpp"

void print_help() {
    std::cout << "./huffman -c|d -h static|adaptive [-m] -i INPUT -o OUTPUT" << std::endl;
}

int main( int argc, char **argv ) {
    int opt, mode = UNSET;
    bool useModel = false;
    std::ifstream infile;
    std::ofstream outfile;
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
                if ( infile.is_open() ) {
                    std::cerr << "Parameter -i is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                infile.open( optarg );
                if ( !infile.is_open() ) {
                    std::cerr << "Unable to open input file" << std::endl;
                    return FILE_ERROR;
                }
                break;
            case 'o':
                if ( outfile.is_open() ) {
                    std::cerr << "Parameter -o is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                outfile.open( optarg );
                if ( !outfile.is_open() ) {
                    std::cerr << "Unable to open output file" << std::endl;
                    return FILE_ERROR;
                }
                break;
            case 'h':
                std::cout << optarg << std::endl;
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
    return 0;
}