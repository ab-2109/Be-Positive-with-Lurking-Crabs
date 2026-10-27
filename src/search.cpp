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
        // get_page_latch returns a shared_pointer to shared_mutex.
        auto latch = BPlusTree::get_page_latch(currentFile);
        currentLock = shared_lock<shared_mutex>(*latch);
        total_s_latches++;
        // we validate if the root node has changed or not.
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

        // the currNode[EMPTY_NODE_VAL] is never really taken because no value can be less than 0.
        // so this implementation can be updated.
        string childFile = (it==currNode.begin()) ? currNode[EMPTY_NODE_VAL] : prev(it)->second;
        auto childLatch = BPlusTree::get_page_latch(childFile);
        shared_lock<shared_mutex> childLock(*childLatch);
        total_s_latches++;
        currentLock.unlock();
        currentFile = childFile; // iterating the tree instead of recursive search.
        currentLock = move(childLock);
    }
}


// old function no really used right now. Can be discarded.
string BPlusTree::f_search(int key, string& file){
    bool isLeaf;
    // files are divided into two parts

    // index nodes have 0 p1 k1 p2 k2 p3...
    // leaf nodes (where data pointers are stored) 1 k1 p1 k2 p2 k3 p3...
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
        // notice we are taking the previous node of it.
        return f_search(key, prev(it)->second);
    }
}