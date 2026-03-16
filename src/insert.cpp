#include "../include/bptree.hpp"

using namespace std;

bool BPlusTree::Insert(int key){
    
    if(Search(key) != ""){
        BP_ERROR = KEY_EXISTS;
        return false;
    }
    if(numRows==0){
        // Making root file
        root = get_free_file_name(false);

        map<int,string> currNode;
        currNode[key] = get_free_file_name(true,key);
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
    newRoot[0] = root;  // According to our map convention, we store the 1st 
    // pointer of the node as value of the key 0 (assuming all key values are 
    // always >0)
    newRoot[node.newFirstKey] = node.newFileName;

    string newRootFile = get_free_file_name(false);

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
        prev(it);

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
            string newFile = get_free_file_name(false);

            split_node(currNode,newNode);

            auto it = newNode.begin();
            int promotedKey = it->first;

            newNode[0] = it->second;
            newNode.erase(it);

            write_file(file, currNode, false);
            write_file(newFile, newNode, false);
            return {true, promotedKey, newFile};
        }
    }
    else{
        currNode[key] = get_free_file_name(true,key);
        if(currNode.size()<=N){
            write_file(file, currNode, true);
            return {false, 0, ""};
        }
        else{
            // Create new file
            map<int,string> newNode;
            string newFile = get_free_file_name(false);
            
            split_node(currNode,newNode);

            write_file(file, currNode, true);
            write_file(newFile, newNode, true);
            return {true, (newNode.begin())->first, newFile};
        }
    }
}