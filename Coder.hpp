#ifndef __CODER
#define __CODER
#include <vector>
#include <iostream>
#include "Tree.hpp"

class Coder {
protected:
    size_t idx;
    size_t byteNotWritten;
    bool useModel;
    bool adaptive;
    void getFileHeaders( std::vector<uint8_t> & lengths, std::vector<uint8_t> & tree, huffmanCode & coder );
    void encode( std::vector<uint8_t> & data, huffmanCode & coder, std::vector<uint8_t> & encoded );
    void staticEncode( std::vector<uint8_t> & data, huffmanCode & coder, FILE * outfile );
    bool staticDecode( std::vector<uint8_t> & data, FILE * outfile );
    void init() {
        this->byteNotWritten = 0;
        this->idx            = 0;
    }
public:
    Coder( bool useModel, bool adaptive ) {
        this->useModel = useModel;
        this->adaptive = adaptive;
        this->init();
    }

    void huffmanEncode( std::vector<uint8_t> data, FILE * output ) {
        this->init();
        std::vector<Tree *> leafs = Tree::loadLeafNodes( data );
        Tree * tree = Tree::buildTree( leafs );
        huffmanCode coder = tree->generateHuffmanCode();
        delete tree;
        this->staticEncode( data, coder, output );
    }

    bool huffmanDecode( std::vector<uint8_t> data, FILE * output ) {
        this->init();
        return this->staticDecode( data, output );
    }
};

#endif