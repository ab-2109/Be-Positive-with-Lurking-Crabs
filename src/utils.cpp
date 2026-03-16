#include "../include/bptree.hpp"

inline void BPlusTree::split_node(map<int,string> &currNode,map<int,string> &newNode)
{
    int k = (N+2)/2;
            auto it = prev(currNode.end(), k);
            
            while(it != currNode.end()){        //splitting current node
                newNode.insert(currNode.extract(it++)); 
    }
}

