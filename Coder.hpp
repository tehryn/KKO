#ifndef __CODER
#define __CODER
#include <vector>
#include <iostream>
#include "Tree.hpp"

enum effort {
    NO_EFFORT = -1,
    EFFORT_6 = 0, // default
    MINIMUM_EFFORT,
    EFFORT_3,
    EFFORT_4,
    EFFORT_5,
    EFFORT_7,
    EFFORT_8,
    EFFORT_9,
};

class Coder {
protected:
    size_t idx;
    size_t byteNotWritten;
    bool useModel;
    bool adaptive;
    int effort;
    void getFileHeaders( std::vector<uint8_t> & lengths, std::vector<uint8_t> & tree, huffmanCode & coder );
    void encode( std::vector<uint8_t> & data, huffmanCode & coder, std::vector<uint8_t> & encoded );
    void staticEncode( std::vector<uint8_t> & data, huffmanCode & coder, FILE * outfile );
    void adaptiveEncode( std::vector<uint8_t> & data, Tree * root, FILE * outfile );
    bool staticDecode( std::vector<uint8_t> & data, FILE * outfile, uint8_t extra, bool model );
    bool adaptiveDecode( std::vector<uint8_t> & data, Tree * root, FILE * outfile, uint8_t extra, bool model, int effort );
    void static data2differences( std::vector<uint8_t> & data );
    void static differences2data( std::vector<uint8_t> & data );
    size_t static nextUpdate( size_t current, int effort );

    void init() {
        this->byteNotWritten = 0;
        this->idx            = 0;
    }
public:
    Coder( bool useModel, bool adaptive, int effort) {
        this->useModel = useModel;
        this->adaptive = adaptive;
        this->effort = effort;
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
        uint8_t extra  = 0b00000111 & settings;
        bool adaptive  = 0b00001000 & settings;
        bool model     = 0b00010000 & settings;
        int  effort    = (0b11100000 & settings) >> 5;
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