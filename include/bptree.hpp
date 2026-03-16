#pragma once

#include <iostream>
#include <fstream>


using namespace std;
#define MAX_ALLOW_ENTRIES 131072

class BPlusTree{
    public:
    static constexpr int N=8;
    static constexpr int MAX_LEVELS=6;
    int numRows;
    int currLevel;

    BPlusTree()
    :numRows(0)
    ,currLevel(1)
    { }




    ~BPlusTree()=default;

    // some new constructor

    void search(int key);
    void insert(int key, string& File);
};