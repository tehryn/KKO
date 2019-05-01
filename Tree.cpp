#include "Tree.hpp"
#include "huffman.hpp"

std::vector<Tree *> Tree::loadLeafNodes( std::vector<uint8_t> data ) {
    std::vector<Tree *> nodes(256);
    uint8_t byte = 0;
    for ( Tree * & node : nodes ) {
        node = new Tree( 0, byte++ );
    }

    for ( uint8_t byte : data ) {
        nodes[ byte ]->count++;
    }

    std::sort( nodes.begin(), nodes.end(), Tree::comparePointers );

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
        Tree * left  = nodes.front();
        nodes.erase( nodes.begin() );
        Tree * right = nodes.front();
        nodes.erase( nodes.begin() );
        root  = new Tree( left->count + right->count, 0, left, right );

        if ( nodes.empty() ) {
            return root;
        }

        bool set = false;
        for( std::vector<Tree *>::iterator it = nodes.begin(); it != nodes.end(); it++ ) {
            if ( root->count <= (*it)->count ) {
                nodes.insert( it, root );
                set = true;
                break;
            }
        }
        if ( !set ) {
            nodes.push_back( root );
        }
    }
    return root;
}

bool Tree::update( std::vector<Tree *> & nodes ) {
    if ( this->parent != nullptr ) {
        Tree * node = nullptr;
        for( Tree * n : nodes ) {
            if ( n->count == this->count && this->depth >= n->depth && this->parent != n && n != this ) {
                node = n;
                break;
            }
        }

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

        this->count++;
        return this->parent->update( nodes ) || ( node != nullptr );
    }
    else {
        this->count++;
        return false;
    }
}