#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <fstream>


using namespace std;
#define MAX_ALLOW_ENTRIES 131072

typedef enum {MEM_FULL, KEY_EXISTS} BP_Error;
BP_Error BP_ERROR;
const map<BP_Error, string> g_ErrorMsg = {
    {MEM_FULL, "Insertion Failed. Memory Limit has been reached."},
    {KEY_EXISTS, "Insertion Failed. The key already exists, to update it, delete it first and then insert again."}
};

typedef struct insert{
    bool didSplit;
    int newFirstKey;
    string newFileName;
} insert_t;


// Helper functionns to read and write to txt files.
template<typename StreamType>
inline void file_opened(const StreamType& file, string_view fileName);

inline void log_error(string_view msg);

map<int,string> read_file(string& fileName, bool& isLeaf);

bool write_file(string& fileName, map<int,string>& currNode, bool isLeaf);

string get_free_file_name(bool isLeaf,int key=-1);



class BPlusTree{
    public:
        static constexpr int N=8;
        static constexpr int MAX_LEVELS=6;
        string root;
        int numRows;
        int currLevel;
        static priority_queue<int> availPointer;

        BPlusTree()
        :numRows(0)
        ,currLevel(0)
        ,root(""){
            for(int i=1; i<=37449; i++){
                availPointer.push(i);
            }
        }
        ~BPlusTree()=default;

        string Search(int key);  // Search wrapper: Returns file name of the found row, "" if not found
        bool Insert(int key);  // Insert wrapper: Returns true on success
        bool Delete(int key);  // Delete wrapper: Returns true on success
    private:
        insert_t f_insert(int key, string& file);  // Recursive insert
        string f_delete(int key, string& file);  // Recursive delete
        string f_search(int key, string& file);  // Recursive search

        inline void split_node(map<int,string> &currNode,map<int,string> &newNode); // split the node in two halfs
        friend string get_free_file_name(bool,int=-1);
};