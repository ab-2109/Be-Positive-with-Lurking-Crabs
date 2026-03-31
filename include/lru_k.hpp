#pragma once
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <deque>
#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <vector>
using namespace std;

#define K (int)2        // this is the value of K for the LRU-K class
#define INDEX_PAGES (int)100
#define DATA_PAGES (int)1000


template<typename T>
class LRU_K{

    static_assert(
        is_same_v<T, map<int, string>> || is_same_v<T, vector<string>>,
        "LRU_K<T> supports only map<int,string> and vector<string>."
    );

    struct Frame {
        T data;
        bool dirty = false;
        bool isLeaf = true; // meaningful only for index pages
        size_t lastAccess = 0;
        deque<size_t> history;
    };

    int K;
    int page;
    size_t accessClock;
    unordered_map<string, Frame> cache;

    void touch(Frame& frame);
    bool evict_if_needed();
    string pick_victim() const;

    bool load_from_disk(const string& fileName, Frame& frame, bool* isLeafOut);
    bool flush_to_disk(const string& fileName, Frame& frame);

    public:

    LRU_K(int kVal,int pageVal)
    :K(kVal)
    ,page(pageVal)
    ,accessClock(0)
    {
        K = max(1, K);
        page = max(0, page);
    }

    ~LRU_K();


    bool is_present(const string& fileName)
    {
        return cache.find(fileName)!=cache.end();
    }

    size_t cache_size() const{
        return cache.size();
    }

    bool read(const string& fileName, T& out, bool* isLeafOut = nullptr);
    bool write(const string& fileName, const T& value, bool isLeaf = true);

    bool flush(const string& fileName);
    void flush_all();

    
};
