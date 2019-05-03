/**
 * File: Tree.hpp
 * Author: Jiri Matejka (xmatej52)
 * Modified: 02. 05. 2019
 * Description: Header file of Tree class where huffman tree is implemented.
 */
#ifndef __TREE
#define __TREE
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>

typedef std::vector<bool> bitSet;
typedef std::map<uint8_t, bitSet> huffmanCode;

class Tree {
public:
    size_t count;  ///< Sum of occurences of char
    uint8_t data;  ///< Data of list
    size_t depth;  ///< Depth of node
    Tree * left;   ///< Left descendant
    Tree * right;  ///< Right descendant
    Tree * parent; ///< Parent

    /**
     * Builds node of tree
     * @param count Sum of occurences of char
     * @param data  Data of list
     * @param left  Left descendant
     * @param right Right descendant
     */
    Tree( size_t count = 0, uint8_t data = 0, Tree * left = nullptr, Tree * right = nullptr ) {
        this->count = count;
        this->data = data;
        this->left = left;
        this->right = right;
        this->parent = nullptr;
        this->depth = 0;
    }

    /**
     * Builds tree from huffman coder
     * @param coder code of tree
     */
    Tree( huffmanCode & coder ) {
        this->count = 0;
        this->data = 0;
        this->depth = 0;
        this->left = nullptr;
        this->right = nullptr;
        this->parent = nullptr;

        // loop over all of bytes
        for( size_t byte = 0; byte < 256; byte++ ) {
            Tree * tmp = this;

            // creates path to specific leaf
            for( bool bit : coder[ byte ] ) {
                if ( bit ) {
                    if ( tmp->right == nullptr ) {
                        tmp->right = new Tree();
                        tmp->right->parent = tmp;
                    }
                    tmp = tmp->right;
                }
                else {
                    if ( tmp->left == nullptr ) {
                        tmp->left = new Tree();
                        tmp->left->parent = tmp;
                    }
                    tmp = tmp->left;
                }
            }

            // init data in leaf
            tmp->data = byte;
        }
    }

    ~Tree() {
        delete this->left;
        delete this->right;
    }

    /**
     * Builds tree from leaves
     * @param  leaves vector of all leaves
     * @return        root of new tree
     */
    static Tree * buildTree( std::vector<Tree *> & leaves );

    /**
     * Builds tree for adaptive encoding or decoding
     * @return root of new tree
     */
    static Tree * initAdaptiveTree();

    /**
     * Creates leaves from input data
     * @param data data from file
     * @return vector of all leaves
     */
    static std::vector<Tree *> loadLeafNodes( std::vector<uint8_t> data );

    /**
     * Sets depth od this node and all his descendants.
     * @param parentValue depth of this node
     */
    void setDepth( size_t parentValue ) {
        this->depth = parentValue++;
        if ( this->left != nullptr ) {
            this->left->setDepth( parentValue );
            this->right->setDepth( parentValue );
        }
    }

    /**
     * Generates huffman coder from given prefix
     * @param prefix prefix of this node
     * @param coder huffman coder
     */
    void generateHuffmanCode( const bitSet & prefix, huffmanCode & coder ) {
        // if leave, then recursion ends
        if ( this->left == nullptr ) {
            coder[ this->data ] = prefix;
        }
        else {
            // set new prefixes for descendants and continue recursion
            bitSet leftPrefix = prefix;
            bitSet rightPrefix = prefix;
            leftPrefix.push_back( false );
            rightPrefix.push_back( true );
            this->left->generateHuffmanCode( leftPrefix, coder );
            this->right->generateHuffmanCode( rightPrefix, coder );
        }
    }

    /**
     * Generate huffman code with no starting prefix (good to use it from node)
     * @return huffman coder
     */
    huffmanCode generateHuffmanCode() {
        huffmanCode coder;
        bitSet prefix;
        this->generateHuffmanCode( prefix, coder );
        return coder;
    }

    /**
     * Rebuild existing huffman coder
     * @param coder huffman coder
     */
    void generateHuffmanCode( huffmanCode & coder ) {
        bitSet prefix;
        this->generateHuffmanCode( prefix, coder );
    }

    /**
     * Reverse implementation of inorder
     * @param vec vector where items will be stored
     */
    void reverseInorder( std::vector<Tree *> & vec ) {
        if ( this->right != nullptr ) {
            this->right->reverseInorder( vec );
            vec.push_back( this );
            this->left->reverseInorder( vec );
        }
        else {
            vec.push_back( this );
        }
    }

    /**
     * Compare cound of two tree pointers
     * @param  left  left operand
     * @param  right right operand
     * @return       true if left is greater then right
     */
    static bool comparePointers( Tree * left, Tree * right ) {
        return left->count > right ->count;
    }

    /**
     * Find node by given huffman code
     * @param  code huffman code of node
     * @return      pointer to node
     */
    Tree * find( bitSet & code ) {
        Tree * seek = this;
        for( bool bit : code ) {
            seek = bit ? seek->right : seek->left;
        }
        return seek;
    }

    /**
     * Update that is used in adaptive implementation - updates node and his parent
     * @param  nodes vector of all nodes in tree
     * @return       true if at least 2 nodes were swapped
     */
    bool update( std::vector<Tree *> & nodes );
};

#endif