#include "../include/bptree.hpp"


string BPlusTree::Search(int key){
    /*This is a wrapper function, which only calls the main recursive one*/
    return search_unlocked(key);
}

string BPlusTree::search_unlocked(int key){
    string currentFile;
    shared_lock<shared_mutex> currentLock;
    while(true){
        {
            lock_guard<mutex> m(metaMutex);
            currentFile = root;
            if (currentFile.empty()) return "";
        }
        auto latch = BPlusTree::get_page_latch(currentFile);
        currentLock = shared_lock<shared_mutex>(*latch);
        total_s_latches++;
        {
            lock_guard<mutex> m(metaMutex);
            if (root == currentFile) break;
        }
        currentLock.unlock();
    }

    while (true) {
        bool isLeaf;
        map<int,string> currNode = read_file(currentFile, isLeaf);
        if(isLeaf){
            auto it = currNode.find(key);
            if(it!=currNode.end()){
                return it->second;
            }
            return "";
        }

        auto it = currNode.upper_bound(key);
        string childFile = (it==currNode.begin()) ? currNode[EMPTY_NODE_VAL] : prev(it)->second;
        auto childLatch = BPlusTree::get_page_latch(childFile);
        shared_lock<shared_mutex> childLock(*childLatch);
        total_s_latches++;
        currentLock.unlock();
        currentFile = childFile;
        currentLock = move(childLock);
    }
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