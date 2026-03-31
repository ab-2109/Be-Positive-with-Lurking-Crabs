#include "../include/bptree.hpp"

BPlusTree::BPlusTree() : root(""), numRows(0), currLevel(0)
{
    if (!load_metadata()) {
        init_fresh_state();
    }
}

BPlusTree::~BPlusTree()
{
    store_metadata();
}

void BPlusTree::init_fresh_state()
{
    root = "";
    numRows = 0;
    currLevel = 0;

    priority_queue<int> empty;
    swap(availPointer, empty);

    for (int i = 1; i <= MAX_INDEX_FILES; i++) {
        availPointer.push(i);
    }
}

bool BPlusTree::load_metadata()
{
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

    priority_queue<int> loadedAvail;
    for (size_t i = 0; i < pqSize; i++) {
        int id = 0;
        if (!(file >> id)) {
            return false;
        }
        loadedAvail.push(id);
    }

    root = loadedRoot;
    numRows = loadedRows;
    currLevel = loadedLevel;
    swap(availPointer, loadedAvail);
    return true;
}

void BPlusTree::store_metadata() const
{
    ofstream file(METADATA_FILE, ios::out | ios::trunc);
    if (!file.is_open()) {
        cerr << "(ERROR) Failed to store metadata in " << METADATA_FILE << endl;
        return;
    }

    file << root << '\n';
    file << numRows << ' ' << currLevel << '\n';

    priority_queue<int> copyQueue = availPointer;
    file << copyQueue.size() << '\n';
    while (!copyQueue.empty()) {
        file << copyQueue.top() << ' ';
        copyQueue.pop();
    }
    file << '\n';
}
