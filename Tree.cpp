/**
 * File: Tree.cpp
 * Author: Jiri Matejka (xmatej52)
 * Modified: 02. 05. 2019
 * Description: Implementation of Tree class where huffman tree is implemented.
 */
#include "Tree.hpp"

std::vector<Tree *> Tree::loadLeafNodes( std::vector<uint8_t> data ) {
    // preallocate memory for nodes
    std::vector<Tree *> nodes(256);

    // inits value of all nodes
    uint8_t byte = 0;
    for ( Tree * & node : nodes ) {
        node = new Tree( 0, byte++ );
    }

    // sets number of occurences of each byte
    for ( uint8_t byte : data ) {
        nodes[ byte ]->count++;
    }

    // sorts vector by occurences
    std::sort( nodes.begin(), nodes.end(), Tree::comparePointers );

    // remove unused nodes
    while( !nodes.empty() && nodes.front()->count == 0  ) {
        Tree * node = nodes.front();
        nodes.erase( nodes.begin() );
        delete node;
    }

    return nodes;
}

Tree * Tree::initAdaptiveTree() {
    std::vector<uint8_t> initVector;
    initVector.reserve( 256 );

    // default values of all chars
    for( int byte = 0; byte < 256; byte++ ) {
        initVector.push_back( byte );
    }

    std::vector<Tree *> initNodes = Tree::loadLeafNodes( initVector );
    return Tree::buildTree( initNodes );
}

Tree * Tree::buildTree( std::vector<Tree *> & nodes ) {
    if ( nodes.size() == 0 ) {
        return nullptr;
    }
    else if ( nodes.size() == 1 ) {
        nodes.clear();
        return new Tree( nodes[0]->count, 0, nodes[0] );
    }

    Tree * root  = nullptr;
    while( true ) {
        // 2 smallest occurences
        Tree * left  = nodes.front();
        nodes.erase( nodes.begin() );
        Tree * right = nodes.front();
        nodes.erase( nodes.begin() );

        // creates new node
        root  = new Tree( left->count + right->count, 0, left, right );

        // set parent
        left->parent = root;
        right->parent = root;

        // current root is root of whole tree
        if ( nodes.empty() ) {
            return root;
        }

        // adds root to sorted vector
        bool set = false;
        for( std::vector<Tree *>::iterator it = nodes.begin(); it != nodes.end(); it++ ) {
            if ( root->count < (*it)->count ) {
                nodes.insert( it, root );
                set = true;
                break;
            }
        }
        if ( !set ) {
            nodes.push_back( root );
        }
    }
    return nullptr;
}

bool Tree::update( std::vector<Tree *> & nodes ) {
    // root cannot be swapped
    if ( this->parent != nullptr ) {
        // finds most right node ind tree with same count and less depth
        Tree * node = nullptr;
        for( Tree * n : nodes ) {
            if ( n->count == this->count && this->depth >= n->depth && this->parent != n && n != this ) {
                node = n;
                break;
            }
        }

        // if node is found, swap it
        if ( node != nullptr ) {
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
        }

        // update count
        this->count++;

        // update parent node
        return this->parent->update( nodes ) || ( node != nullptr );
    }
    else {
        this->count++;
        return false;
    }
}