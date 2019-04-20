#ifndef __CODER
#define __CODER
#include <vector>
#include <iostream>
#include "Tree.hpp"

class Coder {
private:
    size_t idx;
protected:
    bool useModel;
    bool adaptive;
    void staticEncode( std::vector<uint8_t> data, huffmanCode & coder, FILE * outfile );
    bool staticDecode( std::vector<uint8_t> data, FILE * outfile );
public:
    Coder( bool useModel, bool adaptive ) {
        this->useModel = useModel;
        this->adaptive = adaptive;
        this->idx      = 0;
    }

    void encode( std::vector<uint8_t> data, FILE * output ) {
        this->idx = 0;
        std::vector<Tree *> leafs = Tree::loadLeafNodes( data );
        Tree * tree = Tree::buildTree( leafs );
        huffmanCode coder = tree->generateHuffmanCode();
        delete tree;
        this->staticEncode( data, coder, output );
    }

    bool decode( std::vector<uint8_t> data, FILE * output ) {
        this->idx = 0;
        return this->staticDecode( data, output );
    }
};

#endif