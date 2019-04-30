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
    void adaptiveEncode( std::vector<uint8_t> & data, Tree * root, FILE * outfile );
    bool staticDecode( std::vector<uint8_t> & data, FILE * outfile, uint8_t extra );
    bool adaptiveDecode( std::vector<uint8_t> & data, Tree * root, FILE * outfile, uint8_t extra );
    void init() {
        this->byteNotWritten = 0;
        this->idx            = 0;
    }
    static Tree * buildAdaptiveTree() {
        std::vector<Tree *> leaves;
        leaves.reserve( 256 );
        for( int byte = 0; byte < 256; byte++ ) {
            leaves.push_back( new Tree( 1, byte ) );
        }
        return Tree::buildTree( leaves );
    }
public:
    Coder( bool useModel, bool adaptive ) {
        this->useModel = useModel;
        this->adaptive = adaptive;
        this->init();
    }

    void huffmanEncode( std::vector<uint8_t> data, FILE * output ) {
        this->init();
        if ( this->adaptive ) {
            this->adaptiveEncode( data, Coder::buildAdaptiveTree(), output );
        }
        else {
            std::vector<Tree *> leaves = Tree::loadLeafNodes( data );
            Tree * tree = Tree::buildTree( leaves );
            huffmanCode coder = tree->generateHuffmanCode();
            delete tree;
            this->staticEncode( data, coder, output );
        }
    }

    bool huffmanDecode( std::vector<uint8_t> data, FILE * output ) {
        this->init();
        this->idx = 1;
        uint8_t settings = data[0];
        bool adaptive = (1 << 4) & settings;
        uint8_t extra = 7 & settings;
        return adaptive ? this->adaptiveDecode( data, Coder::buildAdaptiveTree(), output, extra ) : this->staticDecode( data, output, extra );
    }
};

#endif