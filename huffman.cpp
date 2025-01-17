/**
 * File: huffman.cpp
 * Author: Jiri Matejka (xmatej52)
 * Modified: 02. 05. 2019
 * Description: Implementation of application where main function and some other usefull functions are implemented
 */
#include "huffman.hpp"

int main( int argc, char **argv ) {
    // program settings
    int opt, mode = UNSET, coding = UNSET, effort = NO_EFFORT;
    bool useModel = false;
    FILE * outfile = nullptr;

    // data from input file
    std::vector<uint8_t> inputData;

    // parse argument
    while ( ( opt = getopt( argc, argv, ":cd0123456789h:mi:o:" ) ) != -1 ) {
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
                break;
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
            case '9':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                else {
                    effort = EFFORT_9;
                }
                break;
            case '8':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                else {
                    effort = EFFORT_8;
                }
                break;
            case '7':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return EFFORT_7;
                }
                else {
                    effort = EFFORT_7;
                }
                break;
            case '6':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                else {
                    effort = EFFORT_6;
                }
                break;
            case '5':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                else {
                    effort = EFFORT_5;
                }
                break;
            case '4':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                else {
                    effort = EFFORT_4;
                }
                break;
            case '3':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                else {
                    effort = EFFORT_3;
                }
                break;
            case '2':
            case '1':
            case '0':
                if ( effort != NO_EFFORT ) {
                    std::cerr << "Parameter effort is set more than once." << std::endl;
                    return INVALID_ARGUMENTS;
                }
                else {
                    effort = MINIMUM_EFFORT;
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
                std::cerr << "unknown option: "<< +optopt << std::endl;
                break;
        }
    }

    // test if all argumets are set
    if ( ( coding == UNSET && mode == COMPRESS ) || mode == UNSET || outfile == nullptr || inputData.size() == 0 ) {
        std::cerr << "Not all program arguments were specified. Run './huffman -h' for help." << std::endl;
        if ( outfile != nullptr ) {
            fclose( outfile );
        }
        return INVALID_ARGUMENTS;
    }

    // Creates huffman coder
    Coder huffmanImp = Coder( useModel, coding == ADAPTIVE, effort == NO_EFFORT ? EFFORT_8 : effort );

    // compress/decompress
    if ( mode == COMPRESS ) {
        huffmanImp.huffmanEncode( inputData, outfile );
    }
    else {
        if ( !huffmanImp.huffmanDecode( inputData, outfile ) ) {
            std::cerr << "Corrupted input file." << std::endl;
            fclose( outfile );
            return CORRUPTED_INPUT;
        }
    }

    fclose( outfile );
    return 0;
}

void print_help() {
    std::cout << "./huffman -c|d -h [static|adaptive] [-m] -i INPUT -o OUTPUT [-0|1|2|3|4|5|6|7|8|9]" << std::endl;
    std::cout << "if -d is set, arguments -h, -m and efforts are ignored" << std::endl;
}

std::vector<uint8_t> readBytes( std::string filename ) {
    std::ifstream file( filename, std::ios::binary );
    return std::vector<uint8_t>( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );
}