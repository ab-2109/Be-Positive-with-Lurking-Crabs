#include "../include/bptree.hpp"

static int real_keys(const map<int, string> &node, bool isLeaf)
{
    return isLeaf ? (int)node.size() : (int)node.size() - 1;
}

static int extract_id(const string &fileName)
{
    // "Index/42.txt" → 42
    size_t slash = fileName.find('/');
    size_t dot = fileName.rfind('.');
    return stoi(fileName.substr(slash + 1, dot - slash - 1));
}

inline void reclaim_file_num(const string& fileName)
{
        remove(fileName.c_str());
        BPlusTree::availPointer.push(extract_id(fileName));
}

inline void BPlusTree::merge_nodes(map<int,string>& left, map<int,string>& right,bool isLeaf, int sepKey)
{
    if (isLeaf) {
        for (auto& [k,v] : right) left[k] = v;
    } else {
        left[sepKey] = right[EMPTY_NODE_VAL];       // sep bridges: right's leftmost child goes under sepKey in left
        for (auto& [k,v] : right)
            if (k != EMPTY_NODE_VAL) left[k] = v;
    }
}


void BPlusTree::borrow_node(map<int,string>& childNode,map<int,string>& sibNode,bool isLeaf, int& sepKey, bool isRightSib)
{
    if (isLeaf) {
        if (isRightSib) {
            childNode[sibNode.begin()->first] = sibNode.begin()->second;  // move sib's first into child
            sibNode.erase(sibNode.begin());
            sepKey = sibNode.begin()->first;                               // new sep = new first of sib
        } else {
            auto borrow = prev(sibNode.end());
            childNode[borrow->first] = borrow->second;                    // move sib's last into child
            sibNode.erase(borrow);
            sepKey = childNode.begin()->first;                             // new sep = new first of child
        }
    } else {
        if (isRightSib) {
            childNode[sepKey]         = sibNode[EMPTY_NODE_VAL];          // sep comes down into child
            auto sibFirst             = next(sibNode.begin());             // sib's first real key goes up
            sibNode[EMPTY_NODE_VAL]   = sibFirst->second;                 // sib's new leftmost child
            sepKey                    = sibFirst->first;                   // new sep = promoted key
            sibNode.erase(sibFirst);
        } else {
            childNode[sepKey]         = childNode[EMPTY_NODE_VAL];        // shift child's leftmost right, under sep
            childNode[EMPTY_NODE_VAL] = prev(sibNode.end())->second;      // sib's last child becomes child's new leftmost
            sepKey                    = prev(sibNode.end())->first;        // new sep = promoted key
            sibNode.erase(prev(sibNode.end()));
        }
    }
}

delete_t BPlusTree::f_delete(int key, const string &file, deque<unique_lock<shared_mutex>>& path_locks)
{
    bool isLeaf;
    map<int, string> currNode = read_file(file, isLeaf);
    const int MIN_KEYS = (N + 1) / 2; // ceil(N/2) = 4 for N=8

    int num_keys = isLeaf ? currNode.size() : currNode.size() - 1;
    bool isSafe = (num_keys > MIN_KEYS);

    if (isSafe && path_locks.size() > 1) {
        while (path_locks.size() > 1) {
            path_locks.front().unlock();
            path_locks.pop_front();
        }
    }

    if (isLeaf)
    {
        auto tgt = currNode.find(key);
        if (tgt == currNode.end()) {
            return {false, false, -1};
        }

        remove(tgt->second.c_str()); // delete the actual row data file
        currNode.erase(tgt);
        write_file(file, currNode, true);

        if (real_keys(currNode, true) >= MIN_KEYS)
            return {true, false, -1}; // healthy — nothing for parent to do

        return {true, true, -1}; // removedSepKey unused at leaf level
    }

    // internal node

    /* this logic is valid as the first index will be zero
    in a map so prev will always be valid */
    auto it = prev(currNode.upper_bound(key));
    string childFile = it->second;

    auto childLatch = BPlusTree::get_page_latch(childFile);
    unique_lock<shared_mutex> childLock(*childLatch); total_x_latches++;
    path_locks.push_back(move(childLock));

    delete_t result = f_delete(key, childFile, path_locks);

    if (!result.success) return result;
    if (!result.didMerge)
        return {true, false, -1};

    // if the child node is deleted then we dont need the seperator key the childfile it was pointing is deleted in recursive call
    // so right not it is dangling
    currNode.erase(result.removedSepKey);

    if (real_keys(currNode, false) >= MIN_KEYS)
    {
        write_file(file, currNode, false);
        return {true, false, -1};
    }

    bool childIsLeaf;
    map<int, string> childNode = read_file(childFile, childIsLeaf);

    it = prev(currNode.upper_bound(key)); // re-navigate with updated currNode

        auto rightIt = next(it);
    bool hasRight = (rightIt != currNode.end());
    auto leftIt   = (it != currNode.begin()) ? prev(it) : currNode.end();
    bool hasLeft  = (leftIt != currNode.end());

    if (hasRight) {
        string sibFile = rightIt->second;
        auto sibLatch = BPlusTree::get_page_latch(sibFile);
        unique_lock<shared_mutex> sibLock(*sibLatch); total_x_latches++;

        bool sibIsLeaf;
        map<int,string> sibNode = read_file(sibFile, sibIsLeaf);
        int sepKey = rightIt->first;

        if (real_keys(sibNode, sibIsLeaf) > MIN_KEYS) {
            int oldSep = sepKey;
            borrow_node(childNode, sibNode, childIsLeaf, sepKey, true);
            currNode[sepKey] = currNode[oldSep];
            currNode.erase(oldSep);
            write_file(childFile, childNode, childIsLeaf);
            write_file(sibFile,   sibNode,   sibIsLeaf);
            write_file(file,      currNode,  false);
            return {true, false, -1};
        }

        merge_nodes(childNode, sibNode, childIsLeaf, sepKey);
        currNode.erase(sepKey);

        reclaim_file_num(sibFile);
        write_file(childFile, childNode, childIsLeaf);
        write_file(file, currNode, false);
        return {true, real_keys(currNode, false) < MIN_KEYS, sepKey};
    }

    if (hasLeft) {
        string sibFile = leftIt->second;
        auto sibLatch = BPlusTree::get_page_latch(sibFile);
        unique_lock<shared_mutex> sibLock(*sibLatch); total_x_latches++;

        bool sibIsLeaf;
        map<int,string> sibNode = read_file(sibFile, sibIsLeaf);
        int sepKey = it->first;

        if (real_keys(sibNode, sibIsLeaf) > MIN_KEYS) {
            int oldSep = sepKey;
            borrow_node(childNode, sibNode, childIsLeaf, sepKey, false);
            currNode[sepKey] = currNode[oldSep];
            currNode.erase(oldSep);
            write_file(sibFile,   sibNode,   sibIsLeaf);
            write_file(childFile, childNode, childIsLeaf);
            write_file(file,      currNode,  false);
            return {true, false, -1};
        }

        merge_nodes(sibNode, childNode, childIsLeaf, sepKey);
        currNode.erase(sepKey);

        reclaim_file_num(childFile);
        write_file(sibFile, sibNode, sibIsLeaf);
        write_file(file, currNode, false);
        return {true, real_keys(currNode, false) < MIN_KEYS, sepKey};
    }

    log_error("delete.cpp(): f_delete: corrupt tree — node has no siblings");
    return {true, false, -1};
}


bool BPlusTree::Delete(int key)
{
    string currentRoot;
    unique_lock<shared_mutex> rootLock;
    
    while(true){
        {
            lock_guard<mutex> m(metaMutex);
            currentRoot = root;
            if (currentRoot.empty()) {
                BP_ERROR=KEY_NOT_EXISTS;
                return false;
            }
        }
        auto latch = BPlusTree::get_page_latch(currentRoot);
        rootLock = unique_lock<shared_mutex>(*latch); total_x_latches++;
        {
            lock_guard<mutex> m(metaMutex);
            if (root == currentRoot) break;
        }
        rootLock.unlock();
    }

    deque<unique_lock<shared_mutex>> path_locks;
    path_locks.push_back(move(rootLock));

    delete_t result = f_delete(key, currentRoot, path_locks);

    if (!result.success) {
        BP_ERROR=KEY_NOT_EXISTS;
        cerr << "(WARNING) Delete(): Key " << key << " not present\n";
        return false;
    }

    numRows.fetch_sub(1);

    if (!result.didMerge)
        return true;

    bool rootIsLeaf;
    map<int, string> rootNode = read_file(currentRoot, rootIsLeaf);

    if (!rootIsLeaf && real_keys(rootNode, false)==EMPTY_NODE_VAL)
    {
        string newRoot=rootNode[EMPTY_NODE_VAL];
        reclaim_file_num(currentRoot);
        {
            lock_guard<mutex> m(metaMutex);
            root = newRoot;
        }
        currLevel.fetch_sub(1); // mirror of currLevel++ in Insert
    }

    if (numRows.load() == 0)
    {
        reclaim_file_num(currentRoot);
        
        {
            lock_guard<mutex> m(metaMutex);
            root = "";
        }
        currLevel.store(0);
    }

    return true;
}