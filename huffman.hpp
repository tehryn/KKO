#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

enum mode {
    UNSET,
    COMPRESS,
    DECOMPRESS
};

enum returnCodes {
    INVALID_ARGUMENTS = 1,
    FILE_ERROR
};

/**
 * Prints help to stdout. Does not ends the program.
 */
void print_help();