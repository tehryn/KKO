#ifndef __TREE
#define __TREE
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>

// vector bool se vytvori jako bitove pole
typedef std::vector<bool> bitSet;
typedef std::map<uint8_t, bitSet> huffmanCode;

class Tree {
public:
    size_t count;
    uint8_t data;
    Tree * left;
    Tree * right;
    Tree( size_t count = 0, uint8_t data = 0, Tree * left = nullptr, Tree * right = nullptr ) {
        this->count = count;
        this->data = data;
        this->left = left;
        this->right = right;
    }

    Tree( huffmanCode & coder ) {
        this->count = 0;
        this->data = 0;
        this->left = nullptr;
        this->right = nullptr;
        for( size_t byte = 0; byte < 256; byte++ ) {
            Tree * tmp = this;
            for( bool bit : coder[ byte ] ) {
                if ( bit ) {
                    if ( tmp->right == nullptr ) {
                        tmp->right = new Tree();
                    }
                    tmp = tmp->right;
                }
                else {
                    if ( tmp->left == nullptr ) {
                        tmp->left = new Tree();
                    }
                    tmp = tmp->left;
                }
            }
            tmp->data = byte;
        }
    }

    ~Tree() {
        delete this->left;
        delete this->right;
    }

    static Tree * buildTree( std::vector<Tree *> leafs );

    static std::vector<Tree *> loadLeafNodes( std::vector<uint8_t> data );

    void generateHuffmanCode( const bitSet & prefix, huffmanCode & coder ) {
        if ( this->left == nullptr && this->right == nullptr ) {
            coder[ this->data ] = prefix;
        }
        else {
            bitSet leftPrefix = prefix;
            bitSet rightPrefix = prefix;
            leftPrefix.push_back( false );
            rightPrefix.push_back( true );
            this->left->generateHuffmanCode( leftPrefix, coder );
            this->right->generateHuffmanCode( rightPrefix, coder );
        }
    }

    huffmanCode generateHuffmanCode() {
        huffmanCode coder;
        bitSet prefix;
        this->generateHuffmanCode( prefix, coder );
        return coder;
    }

    static bool comparePointers( Tree * left, Tree * right ) {
        return left->count > right ->count;
    }

    friend std::ostream & operator << ( std::ostream & stream, const Tree & node) {
        bool leaf = node.left == nullptr && node.right == nullptr;
        if ( node.left != nullptr ) {
            stream << *(node.left);
        }

        /*if ( leaf ) {
            stream << "[ d:" << +node.data << ", c:" << node.count << " ]";
        }
        else {
            stream << "[ c:" << node.count << " ]";
        }*/

        if ( leaf ) {
            stream << "[ d:" << +node.data <<  " ]";
        }
        else {
            stream << "(-OOO-)";
        }

        if ( node.right != nullptr ) {
            stream << *(node.right);
        }

        return stream;
    }

};

#endif