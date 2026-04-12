#include "../include/bptree.hpp"

unordered_map<string, shared_ptr<shared_mutex>> BPlusTree::pageLatches;
mutex BPlusTree::pageLatchMutex;
mutex BPlusTree::metaMutex;
atomic<long long> BPlusTree::total_s_latches{0};
atomic<long long> BPlusTree::total_x_latches{0};

shared_ptr<shared_mutex> BPlusTree::get_page_latch(const string& fileName){
    lock_guard<mutex> lock(pageLatchMutex);
    auto it = pageLatches.find(fileName);
    if (it != pageLatches.end()) {
        return it->second;
    }
    auto latch = make_shared<shared_mutex>();
    pageLatches[fileName] = latch;
    return latch;
}

BPlusTree::BPlusTree() : root(""), numRows(0), currLevel(0), indexCache(K, INDEX_PAGES) {
    if (!load_metadata()) init_fresh_state();
}

BPlusTree::~BPlusTree(){
    store_metadata();
    print_latch_stats();
}

void BPlusTree::print_latch_stats() {
    cout << "\n-------- B+ Tree Latch Statistics --------\n";
    cout << "Total S-Latches (Tree): " << total_s_latches.load() << "\n";
    cout << "Total X-Latches (Tree): " << total_x_latches.load() << "\n";
    cout << "Total Unique Page Latches Created: " << pageLatches.size() << "\n";
    cout << "------------------------------------------\n";
}

void BPlusTree::init_fresh_state(){
    lock_guard<mutex> lock(metaMutex);
    root = "";
    numRows.store(0);
    currLevel.store(0);

    priority_queue<int, vector<int>, greater<int>> empty;
    swap(availPointer, empty);

    for (int i = 1; i <= MAX_INDEX_FILES; i++) {
        availPointer.push(i);
    }
}

bool BPlusTree::load_metadata(){
    lock_guard<mutex> lock(metaMutex);
    if (!filesystem::exists(METADATA_FILE)) {
        return false;
    }

    ifstream file(METADATA_FILE, ios::in);
    if (!file.is_open()) {
        return false;
    }

    string loadedRoot;
    if (!getline(file, loadedRoot)) {
        return false;
    }

    int loadedRows = 0;
    int loadedLevel = 0;
    size_t pqSize = 0;
    if (!(file >> loadedRows >> loadedLevel)) {
        return false;
    }
    if (!(file >> pqSize)) {
        return false;
    }

    priority_queue<int, vector<int>, greater<int>> loadedAvail;
    for (size_t i = 0; i < pqSize; i++) {
        int id = 0;
        if (!(file >> id)) {
            return false;
        }
        loadedAvail.push(id);
    }

    root = loadedRoot;
    numRows.store(loadedRows);
    currLevel.store(loadedLevel);
    swap(availPointer, loadedAvail);
    return true;
}

void BPlusTree::store_metadata() const{
    lock_guard<mutex> lock(metaMutex);
    ofstream file(METADATA_FILE, ios::out | ios::trunc);
    if (!file.is_open()) {
        cerr << "(ERROR) Failed to store metadata in " << METADATA_FILE << endl;
        return;
    }

    file << root << '\n';
    file << numRows.load() << ' ' << currLevel.load() << '\n';

    priority_queue<int, vector<int>, greater<int>> copyQueue = availPointer;
    file << copyQueue.size() << '\n';
    while (!copyQueue.empty()) {
        file << copyQueue.top() << ' ';
        copyQueue.pop();
    }
    file << '\n';
}
