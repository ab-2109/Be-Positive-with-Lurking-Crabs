#include "../include/bptree.hpp"


inline void BPlusTree::split_node(map<int,string> &currNode,map<int,string> &newNode)
{
    int k = (N+2)/2;
            auto it = prev(currNode.end(), k);
            
            while(it != currNode.end()){        //splitting current node
                newNode.insert(currNode.extract(it++));  // it++ here is to prevent iterator invalidation
    }
}


bool BPlusTree::Insert(int key){
    string currentRoot;
    unique_lock<shared_mutex> rootLock;

    while(true){
        {
            lock_guard<mutex> m(metaMutex);
            currentRoot = root;
            if (currentRoot.empty()) {
                // Tree is empty


                // get_free_file_name is not synchronised.

                // need to change the false to Data and Index types better semantics.
                root = get_free_file_name(false);

                map<int,string> currNode;
                currNode[key] = get_free_file_name(true,key);
                write_file(root, currNode, true); // here need to add LEAF/NON LEAF Type.
                numRows.fetch_add(1);
                currLevel.fetch_add(1);
                return true;
            }
        }
        auto latch = BPlusTree::get_page_latch(currentRoot);
        rootLock = unique_lock<shared_mutex>(*latch); 
        total_x_latches++;
        {
            lock_guard<mutex> m(metaMutex);
            if (root == currentRoot) break;
        }
        rootLock.unlock();
    }

    // this check is not atomic has to make it atomic.
    if(numRows.load() == MAX_ALLOW_ENTRIES){
        BP_ERROR = MEM_FULL;
        return false;
    }

    numRows.fetch_add(1);

    deque<unique_lock<shared_mutex>> path_locks;
    path_locks.push_back(move(rootLock));

    insert_t node = f_insert(key, currentRoot, path_locks);
    if(!node.success){
        BP_ERROR = KEY_EXISTS;
        numRows.fetch_sub(1); 
        return false;
    }
    if(!node.didSplit) return true;

    // Time to create new root
    // the main reason we required two insert functions insert and f_insert.


    // According to our map convention, we store the 1st 
    // pointer of the node as value of the key 0 (assuming all key values are 
    // always >0)
    map<int,string> newRoot;
    newRoot[EMPTY_NODE_VAL] = currentRoot;  
    newRoot[node.newFirstKey] = node.newFileName;

    string newRootFile = get_free_file_name(false);

    write_file(newRootFile, newRoot, false);  // New root is not leaf
    {
        lock_guard<mutex> m(metaMutex);
        root = newRootFile; 
    }
    currLevel.fetch_add(1);

    return true;
}

insert_t BPlusTree::f_insert(int key, const string& file, deque<unique_lock<shared_mutex>>& path_locks){
    bool isLeaf;
    map<int,string> currNode = read_file(file, isLeaf);

    bool isSafe = (currNode.size() < N); // this checks if after inserting are there chances that node split
    if (isSafe && path_locks.size() > 1) { // if it won't split and path lock contains parent nodes then unlock them.
        while (path_locks.size() > 1) {
            path_locks.front().unlock();
            path_locks.pop_front();
        }
    }

    if(!isLeaf){
        // In the map, search for the key
        auto it = prev(currNode.upper_bound(key));
        string childFile = it->second;

        auto childLatch = BPlusTree::get_page_latch(childFile);
        unique_lock<shared_mutex> childLock(*childLatch); total_x_latches++;
        path_locks.push_back(move(childLock));

        insert_t node = f_insert(key, childFile, path_locks);

        if(!node.success) return node;
        if(!node.didSplit) return node;
        
        currNode[node.newFirstKey] = node.newFileName;
        if(currNode.size()<=N){
            write_file(file, currNode, false);
            return {true, false, 0, ""};
        }
        else{
            // Create new file
            map<int,string> newNode;
            string newFile = get_free_file_name(false);

            split_node(currNode,newNode);

            auto it = newNode.begin();
            int promotedKey = it->first; // this key will become the pivot key in the parent index node. for the new node.

            // here we are doing this because after split the new node has non-empty-node val as it's first value
            // but our convention says that first node has to be 0 in Index node.
            newNode[EMPTY_NODE_VAL] = it->second;
            newNode.erase(it);

            write_file(file, currNode, false);
            write_file(newFile, newNode, false);
            return {true, true, promotedKey, newFile};
        }
    }
    else{
        if (currNode.find(key) != currNode.end()) {
            return {false, false, 0, ""};
        }
        currNode[key] = get_free_file_name(true,key);
        if(currNode.size()<=N){
            write_file(file, currNode, true);
            return {true, false, 0, ""};
        }
        else{
            // Create new file
            map<int,string> newNode;
            string newFile = get_free_file_name(false);
            
            split_node(currNode,newNode);

            // here we don't erase or replace the first value of map with 0
            // as per our convention here the first node can be non-zero

            write_file(file, currNode, true);
            write_file(newFile, newNode, true);
            return {true, true, (newNode.begin())->first, newFile};
        }
    }
}