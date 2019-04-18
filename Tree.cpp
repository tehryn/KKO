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

Tree * Tree::buildTree( std::vector<Tree *> nodes ) {
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