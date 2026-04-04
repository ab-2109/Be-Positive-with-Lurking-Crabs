#pragma once
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <deque>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <vector>
#include <mutex>
#include <atomic>
using namespace std;

extern int K;
extern int INDEX_PAGES;
extern int DATA_PAGES;


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
        mutable mutex frameMutex;
    };

    int k_param;
    int page;
    size_t accessClock;
    unordered_map<string, Frame> cache;
    mutable mutex cacheMapMutex;
    
    // Statistics tracking
    long long totalReads = 0;
    long long totalWrites = 0;
    long long readHits = 0;
    long long writeHits = 0;

    mutable atomic<long long> map_latches{0};
    mutable atomic<long long> frame_latches{0};

    void touch(Frame& frame);
    bool evict_if_needed();
    string pick_victim() const;

    bool load_from_disk(const string& fileName, Frame& frame, bool* isLeafOut);
    bool flush_to_disk(const string& fileName, Frame& frame);

    public:

    LRU_K(int kVal,int pageVal)
    :k_param(kVal)
    ,page(pageVal)
    ,accessClock(0)
    {
        k_param = max(1, k_param);
        page = max(0, page);
    }

    ~LRU_K();


    bool is_present(const string& fileName)
    {
        lock_guard<mutex> lock(cacheMapMutex); map_latches++;
        return cache.find(fileName)!=cache.end();
    }

    size_t cache_size() const{
        lock_guard<mutex> lock(cacheMapMutex); map_latches++;
        return cache.size();
    }

    bool read(const string& fileName, T& out, bool* isLeafOut = nullptr);
    bool write(const string& fileName, const T& value, bool isLeaf = true);

    bool flush(const string& fileName);
    void flush_all();
    void print_stats() const;

    
};
