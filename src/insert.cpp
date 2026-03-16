#include "include/bptree.hpp"

using namespace std;

bool BPlusTree::Insert(int key){
    if(numRows==0){
        // Making root file
        root = to_string(availPointer.top())+".txt";
        availPointer.pop();

        map<int,string> currNode;
        currNode[key] = to_string(key)+".txt";
        bool status = write_file(root, currNode, true);
        return status;
    }
    else if(numRows == MAX_ALLOW_ENTRIES){
        return false;
    }
    insert_t node = f_insert(key, root);
    return true;
}

insert_t BPlusTree::f_insert(int key, string& file){
    bool isLeaf;
    map<int,string> currNode = read_file(file, isLeaf);
    if(!isLeaf){
        // In the map, search for the key
        auto it = currNode.upper_bound(key);
        it--;
        insert_t node = f_insert(key, it->second);
        //do
    }
    else{
        currNode[key] = to_string(key)+".txt";
        if(currNode.size()<=N){
            write_file(root, currNode, true);
            return {false, 0, ""};
        }
        else{
            // Create new file
            map<int,string> newNode;
            string newFile = to_string(availPointer.top())+".txt";
            availPointer.pop();

            int k = (N+2)/2;
            auto it = currNode.end();
            for(int i=0; i<k; i++){  // Splitting current node
                --it;
                newNode[it->first] = it->second;
            }
            currNode.erase(it, currNode.end());

            write_file(file, currNode, true);
            write_file(newFile, newNode, true);
            return {true, (newNode.begin())->first, newFile};
        }
    }
}