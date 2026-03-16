#include "include/bptree.hpp"

using namespace std;

bool BPlusTree::Insert(int key){
    if(BPlusTree::Search(key) != ""){
        BP_ERROR = KEY_EXISTS;
        return false;
    }
    if(numRows==0){
        // Making root file
        root = to_string(availPointer.top())+".txt";
        availPointer.pop();

        map<int,string> currNode;
        currNode[key] = to_string(key)+".txt";
        bool status = write_file(root, currNode, true);
        if(status){
            numRows++;
            currLevel++;
        }
        return status;
    }
    else if(numRows == MAX_ALLOW_ENTRIES){
        BP_ERROR = MEM_FULL;
        return false;
    }

    numRows++;

    insert_t node = f_insert(key, root);
    if(!(node.didSplit)) return true;

    // Time to create new root
    map<int,string> newRoot;

    string newRootFile = to_string(availPointer.top())+".txt";
    availPointer.pop();

    newRoot[0] = root;  // According to our map convention, we store the 1st 
    // pointer of the node as value of the key 0 (assuming all key values are 
    // always >0)
    newRoot[node.newFirstKey] = node.newFileName;
    write_file(newRootFile, newRoot, false);  // New root is not leaf
    root = newRootFile;
    currLevel++;

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

        if(!(node.didSplit)) return node;
        
        currNode[node.newFirstKey] = node.newFileName;
        if(currNode.size()<=N){
            write_file(file, currNode, false);
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

            int promotedKey = newNode.begin()->first;
            newNode[0] = newNode.begin()->second;
            newNode.erase(newNode.begin());

            write_file(file, currNode, false);
            write_file(newFile, newNode, false);
            return {true, promotedKey, newFile};
        }
    }
    else{
        currNode[key] = to_string(key)+".txt";
        if(currNode.size()<=N){
            write_file(file, currNode, true);
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