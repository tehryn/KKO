#include "Tree.hpp"

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
    int byte = 0;
    while ( byte < 256 ) {
        initVector.push_back( byte++ );
    }
    std::vector<Tree *> initNodes = Tree::loadLeafNodes( initVector );
    return Tree::buildTree( initNodes );
}

Tree * Tree::buildTree( std::vector<Tree *> & leaves, std::vector<Tree *> * nodes ) {
    if ( leaves.size() == 0 ) {
        return nullptr;
    }
    else if ( leaves.size() == 1 ) {
        Tree * tmp;
        if ( nodes != nullptr && nodes->size() > 0 ) {
            tmp = nodes->back();
            nodes->pop_back();
            *tmp = *leaves[0];
        }
        else {
            tmp = new Tree( leaves[0]->count, 0, leaves[0] );
        }
        return tmp;
    }

    Tree * root  = nullptr;
    while( true ) {
        Tree * left  = leaves.front();
        leaves.erase( leaves.begin() );
        Tree * right = leaves.front();
        leaves.erase( leaves.begin() );
        if ( nodes != nullptr && nodes->size() > 0 ) {
            root = nodes->back();
            nodes->pop_back();
            root->count = left->count + right->count;
            root->data  = 0;
            root->left  = left;
            root->right = right;
        }
        else {
            root  = new Tree( left->count + right->count, 0, left, right );
        }
        root->left->parent = root->right->parent = root;

        if ( leaves.empty() ) {
            return root;
        }

        bool set = false;
        for( std::vector<Tree *>::iterator it = leaves.begin(); it != leaves.end(); it++ ) {
            if ( root->count <= (*it)->count ) {
                leaves.insert( it, root );
                set = true;
                break;
            }
        }
        if ( !set ) {
            leaves.push_back( root );
        }
    }

    return root;
}