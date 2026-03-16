#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <fstream>


using namespace std;
#define MAX_ALLOW_ENTRIES 131072

priority_queue gAvailPointer;

typedef struct{
    bool did_split;
    int new_first_key;
    string new_file_name;
} insert_t;

class BPlusTree{
    public:
        static constexpr int N=8;
        static constexpr int MAX_LEVELS=6;
        string root;
        int numRows;
        int currLevel;

        BPlusTree()
        :numRows(0)
        ,currLevel(1)
        { }




        ~BPlusTree()=default;

        // some new constructor

        bool Search(int key);
        bool Insert(int key);
        bool Delete(int key);
    private:
        insert_t f_insert(int key, string& file);
        ... f_delete(int key, string& file);
        string f_search(int key, string& file);
};