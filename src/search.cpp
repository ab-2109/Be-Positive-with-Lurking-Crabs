#include "../include/bptree.hpp"


string BPlusTree::Search(int key){
    /*This is a wrapper function, which only calls the main recursive one*/
    if(root==""){
        return "";
    }
    return f_search(key,root);
}

string BPlusTree::f_search(int key, string& file){
    bool isLeaf;
    map<int,string> currNode = read_file(file, isLeaf);
    if(isLeaf){
        auto it = currNode.find(key);
        if(it!=currNode.end()){
            return it->second;
        }else{
            return "";
        }
    }
    auto it = currNode.upper_bound(key);
    if(it==currNode.begin()){
        return f_search(key, currNode[EMPTY_NODE_VAL]);
    }else{
        return f_search(key, prev(it)->second);
    }
}