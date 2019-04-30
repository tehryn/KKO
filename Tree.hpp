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
    uint8_t depth;
    Tree * left;
    Tree * right;
    Tree * parent;
    Tree( size_t count = 0, uint8_t data = 0, Tree * left = nullptr, Tree * right = nullptr ) {
        this->count = count;
        this->data = data;
        this->left = left;
        this->right = right;
        this->parent = nullptr;
        this->depth = 0;
    }

    Tree( huffmanCode & coder ) {
        this->count = 0;
        this->data = 0;
        this->depth = 0;
        this->left = nullptr;
        this->right = nullptr;
        this->parent = nullptr;
        for( size_t byte = 0; byte < 256; byte++ ) {
            Tree * tmp = this;
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
            tmp->data = byte;
        }
    }

    ~Tree() {
        delete this->left;
        delete this->right;
    }

    static Tree * buildTree( std::vector<Tree *> & leaves, std::vector<Tree *> * nodes = nullptr );

    static Tree * initAdaptiveTree();

    static std::vector<Tree *> loadLeafNodes( std::vector<uint8_t> data );

    void setDepth( uint8_t parentValue ) {
        this->depth = parentValue + 1;
        if ( this->left != nullptr ) {
            this->left->setDepth( this->depth );
            this->right->setDepth( this->depth );
        }
    }

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


    static bool comparePointers( Tree * left, Tree * right ) {
        return left->count > right ->count;
    }

    friend std::ostream & operator << ( std::ostream & stream, const Tree & node) {
        bool leaf = node.left == nullptr;
        if ( !leaf ) {
            stream << *(node.left);
            stream << "[ c:" << node.count << " ]";
            stream << *(node.right);
        }
        else {
            stream << "[ d:" << +node.data << ", c:" << node.count << " ]";

        }

        return stream;
    }


    void getLeaves( std::vector<Tree *> & leaves, std::vector<Tree *> & nodes ) {
        if ( this->left != nullptr ) {
            this->right->getLeaves( leaves, nodes );
            this->left->getLeaves( leaves, nodes );
            nodes.push_back( this );
        }
        else {
            leaves.push_back( this );
        }
    }

    Tree * find( bitSet & code ) {
        Tree * seek = this;
        for( bool bit : code ) {
            seek = bit ? seek->right : seek->left;
        }
        return seek;
    }

    bool update( std::vector<Tree *> nodes ) {
        if ( this->parent != nullptr ) {
            bool modified = false;
            for( Tree * node : nodes ) {
                if ( node->count == this->count && this->depth >= node->depth && this->parent != node && node != this ) {
                    modified = true;
                    if ( this == this->parent->left ) {
                        this->parent->left = node;
                    }
                    else {
                        this->parent->right = node;
                    }

                    if ( node == node->parent->left ) {
                        node->parent->left = this;
                    }
                    else {
                        node->parent->right = this;
                    }

                    std::swap( this->parent, node->parent );
                    std::swap( this->depth, node->depth );
                    node->setDepth( node->depth );
                    this->setDepth( this->depth );
                    break;
                }
            }

            this->count++;
            Tree * root = this->parent == nullptr ? this : this->parent;
            while ( root->parent != nullptr ) {
                root = root->parent;
            }
            nodes.clear();
            root->reverseInorder( nodes );

            return this->parent->update( nodes ) || modified;
        }
        else {
            this->count++;
            return false;
        }
    }
};

#endif