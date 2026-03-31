#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <fstream>
#include <filesystem>
#include "lru_k.hpp"


using namespace std;
#define MAX_ALLOW_ENTRIES 131072
#define EMPTY_NODE_VAL 0

typedef enum {MEM_FULL, KEY_EXISTS, KEY_NOT_EXISTS} BP_Error;
inline BP_Error BP_ERROR = MEM_FULL;
inline const map<BP_Error, string> g_ErrorMsg = {
    {MEM_FULL, "Insertion Failed. Memory Limit has been reached."},
    {KEY_EXISTS, "Insertion Failed. The key already exists, to update it, delete it first and then insert again."},
    {KEY_NOT_EXISTS,"Delete Failed. The key could not found."}
};

typedef struct insert{
    bool didSplit;
    int newFirstKey;
    string newFileName;
} insert_t;


typedef struct deletion {
    bool didMerge;       // mirror of didSplit
    int  removedSepKey;  // the separator key the parent must drop
} delete_t;

// Helper functionns to read and write to txt files.
template<typename StreamType>
inline void file_opened(const StreamType& file, string_view fileName)
{
    if (!file.is_open())
    {
        cerr << "(ERROR) file_ops.cpp: Can't Open file " << fileName << endl;
        exit(EXIT_FAILURE);
    }
}

void log_error(string_view msg);
string get_free_file_name(bool isData,int key=-1);



class BPlusTree{
    public:
        static constexpr int N=8;
        static constexpr int MAX_LEVELS=6;
        static constexpr int MAX_INDEX_FILES = 37449;
        static constexpr const char* METADATA_FILE = "metadata.txt";
        string root;
        int numRows;
        int currLevel;
        static priority_queue<int, vector<int>, greater<int>> availPointer;

        BPlusTree();
        ~BPlusTree();

        string Search(int key);  // Search wrapper: Returns file name of the found row, "" if not found
        bool Insert(int key);  // Insert wrapper: Returns true on success
        bool Delete(int key);  // Delete wrapper: Returns true on success
    private:
        insert_t f_insert(int key, string& file);  // Recursive insert
        delete_t f_delete(int key, string& file);  // Recursive delete
        string f_search(int key, string& file);  // Recursive search

        inline void split_node(map<int,string> &currNode,map<int,string> &newNode); // split the node in two halfs
        inline void merge_nodes(map<int,string>& left, map<int,string>& right,bool isLeaf, int sepKey);
        void borrow_node(map<int,string>& childNode,map<int,string>& sibNode,bool isLeaf, int& sepKey, bool isRightSib);
        void init_fresh_state();
        bool load_metadata();
        void store_metadata() const;

        map<int,string> read_file(const string& fileName, bool& isLeaf);
        bool write_file(const string& fileName, map<int,string>& currNode, bool isLeaf);
        LRU_K<map<int, string>> indexCache;  // Cache for index nodes
        
        friend string get_free_file_name(bool,int);
};