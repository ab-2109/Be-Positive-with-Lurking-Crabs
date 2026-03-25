#pragma once
#include <iostream>
#include <unordered_map>
#include <algorithm>
using namespace std;


template<typename T>
class LRU_K{

    int K;
    int page;
    unordered_map<string,T> cache;

    public:

    LRU_K(int kVal,int pageVal)
    :K(kVal)
    ,page(pageVal)
    {

    }


    bool is_present(const string& fileName)
    {
        return cache.find(fileName)!=cache.end();
    }

    size_t cache_size(){
        return cache.size();
    }

    
};
