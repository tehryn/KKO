/**
 * File: Coder.hpp
 * Author: Jiri Matejka (xmatej52)
 * Modified: 02. 05. 2019
 * Description: Header file of Coder class where huffman encoding/decoding is implemented.
 */
#ifndef __CODER
#define __CODER
#include <vector>
#include <iostream>
#include "Tree.hpp"

enum effort {
    NO_EFFORT = -1,
    EFFORT_6 = 0,
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
    size_t idx;             ///< index that share encoding methods
    size_t byteNotWritten;  ///< modified byte that was not written into output
    bool useModel;          ///< if model should be used during encoding, ignored during decoding
    bool adaptive;          ///< if adaptive encoding should be performed, ignored during decoding
    int effort;             ///< how much effort should adaptive encoding put into process, ignored during decoding

    /**
     * Header of file for static encoding
     * @param lengths vector lengths of trees will be stored
     * @param tree    vector where huffman tree will be stored (last element might not be stored here, it will be stored in this->byteNotWritten)
     * @param coder   coded huffman tree
     */
    void getFileHeaders( std::vector<uint8_t> & lengths, std::vector<uint8_t> & tree, huffmanCode & coder );

    /**
     * Encode data into encoded vector
     * @param data    data that shall be encoded
     * @param coder   coder that shall be used
     * @param encoded vector where result will be stored
     */
    void encode( std::vector<uint8_t> & data, huffmanCode & coder, std::vector<uint8_t> & encoded );

    /**
     * implementation of static encoder
     * @param data    data that shall be encoded
     * @param coder   coder that shall be used
     * @param outfile file where result will we written
     */
    void staticEncode( std::vector<uint8_t> & data, huffmanCode & coder, FILE * outfile );

    /**
     * implementation of adaptive encoding
     * @param data    data that shall be encoded
     * @param root    tree that shall be used and updated
     * @param outfile output file where result will be written
     */
    void adaptiveEncode( std::vector<uint8_t> & data, Tree * root, FILE * outfile );

    /**
     * Implementation of static decoder
     * @param  data    encoded data
     * @param  outfile output file where original data will be written
     * @param  extra   number of extra bits in the end of file, that shall be ignored
     * @param  model   tells if model was used during encoding
     * @return         true on succes, false when data are corrupted
     */
    bool staticDecode( std::vector<uint8_t> & data, FILE * outfile, uint8_t extra, bool model );

    /**
     * Implementation of adaptive decoder
     * @param  data    encoded data
     * @param  root    init value of tree
     * @param  outfile output file where result will be written
     * @param  extra   number of extra bits in the end of file, that shall be ignored
     * @param  model   tells if model was used during encoding
     * @param  effort  tells how much efforts were given during encoding
     * @return         true on succes, false when data are corrupted
     */
    bool adaptiveDecode( std::vector<uint8_t> & data, Tree * root, FILE * outfile, uint8_t extra, bool model, int effort );

    /**
     * Implementation of model -- convert data into differences of byte values
     * @param  data input data and also vector where result will be stored
     */
    void static data2differences( std::vector<uint8_t> & data );

    /**
     * Implementation of model -- convert differences into original data
     * @param  data differences between bytes and also vecotr where result will be stored
     */
    void static differences2data( std::vector<uint8_t> & data );

    /**
     * Counts when tree should be updated again
     * @param  current current parsed bytes
     * @param  effort  effort used during compression
     * @return         balue of parsed bytes when tree should be updated again
     */
    size_t static nextUpdate( size_t current, int effort );

    /**
     * Initialize class before every encoding/decoding
     */
    void init() {
        this->byteNotWritten = 0;
        this->idx            = 0;
    }
public:
    /**
     * Creates obejct of coder for encoding or decoding data
     * @param useModel if true, model will be used during encoding. Ignored during decoding
     * @param adaptive if true, adaptiv encoding will be performed. Ignored duting decoding
     * @param effort   sets ho much effort will be givven during adaptive envoding, ignored during decoding
     */
    Coder( bool useModel, bool adaptive, int effort) {
        this->useModel = useModel;
        this->adaptive = adaptive;
        this->effort = effort;
        this->init();
    }

    /**
     * Prepare settings for encoding and then perform encoding
     * @param data   data that shall be encoded
     * @param output output file where result will be written
     */
    void huffmanEncode( std::vector<uint8_t> & data, FILE * output );

    /**
     * Loads encoding settings from encoded data and perform decoding
     * @param  data   encoded data
     * @param  output output file where original data will be stored
     * @return        true if decoding succeded, false if data are corrupted
     */
    bool huffmanDecode( std::vector<uint8_t> & data, FILE * output );
};

#endif