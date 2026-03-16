#pragma once

#include <iostream>
#include <fstream>


using namespace std;

static constexpr int N=8;

class BPlusTree{


    public:

    BPlusTree()=default;
    ~BPlusTree()=default;

    // some new constructor

    void search(int key);
    void insert(int key, string& File);


};