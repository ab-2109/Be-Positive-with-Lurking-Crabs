#include "../include/bptree.hpp"



// this returns true if it doesnot find key in the tree
bool BPlusTree::Delete(int key)
{
    if(Search(key)=="")
    {
        cerr<<"(WARNING) Delete.cpp: Delete(): Key Not Present\n";
        return true;
    }

    // more on this coming soon. Stay tuned.....


}