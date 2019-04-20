#ifndef __HUFFMAN
#define __HUFFMAN
#include <iostream>
#include <fstream>
#include <vector>

enum programSettings {
    UNSET,
    COMPRESS,
    DECOMPRESS,
    STATIC,
    ADAPTIVE
};

enum returnCodes {
    INVALID_ARGUMENTS = 1,
    FILE_ERROR,
    CORRUPTED_INPUT
};

/**
 * Prints help to stdout. Does not ends the program.
 */
void print_help();

/**
 * Reads file content into vector.
 * @param filename name of file
 * @return vector of bytes.
 */
std::vector<uint8_t> readBytes( std::string filename );

template<typename T1, typename T2>
void DEBUG_INLINE(T1 x, T2 y) {
    std::cerr << x << y;
}

template<typename T1, typename T2>
void DEBUG_LINE(T1 x, T2 y) {
    std::cerr << x << y << std::endl;
}

template<typename T1, typename T2, typename T3>
void DEBUG_LINE(T1 x, T2 y, T3 z) {
    std::cerr << x << y << z << std::endl;
}

template<typename T1, typename T2, typename T3>
void DEBUG_INLINE(T1 x, T2 y, T3 z) {
    std::cerr << x << y << z;
}
#endif