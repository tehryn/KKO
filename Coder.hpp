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
    bool staticDecode( std::vector<uint8_t> & data, FILE * outfile, uint8_t extra, bool model );
    bool adaptiveDecode( std::vector<uint8_t> & data, Tree * root, FILE * outfile, uint8_t extra, bool model, bool effort );
    void static data2differences( std::vector<uint8_t> & data );
    void static differences2data( std::vector<uint8_t> & data );

    void init() {
        this->byteNotWritten = 0;
        this->idx            = 0;
    }
public:
    Coder( bool useModel, bool adaptive) {
        this->useModel = useModel;
        this->adaptive = adaptive;
        this->init();
    }

    void huffmanEncode( std::vector<uint8_t> & data, FILE * output ) {
        this->init();
        if ( this->useModel ) {
            Coder::data2differences( data );
        }
        if ( this->adaptive ) {
            Tree * tree = Tree::initAdaptiveTree();
            this->adaptiveEncode( data, tree, output );
            delete tree;
        }
        else {
            std::vector<Tree *> leaves = Tree::loadLeafNodes( data );
            Tree * tree = Tree::buildTree( leaves );
            huffmanCode coder = tree->generateHuffmanCode();
            delete tree;
            this->staticEncode( data, coder, output );
        }
    }

    bool huffmanDecode( std::vector<uint8_t> & data, FILE * output ) {
        this->init();
        this->idx = 1;
        uint8_t settings = data[0];
        bool adaptive = (1 << 4) & settings;
        bool model    = (1 << 5) & settings;
        bool effort   = (1 << 7) & settings;
        uint8_t extra = 7 & settings;
        if ( adaptive ) {
            Tree * tree = Tree::initAdaptiveTree();
            bool ret = this->adaptiveDecode( data, Tree::initAdaptiveTree(), output, extra, model, effort );
            delete tree;
            return ret;
        }
        else {
            return this->staticDecode( data, output, extra, model );
        }
    }
};

#endif