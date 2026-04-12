#include "../include/lru_k.hpp"

template <typename T>
LRU_K<T>::~LRU_K() {
    flush_all();
    print_stats();
}

template <typename T>
void LRU_K<T>::touch(Frame& frame) {
    ++accessClock;
    frame.lastAccess = accessClock;
    frame.history.push_back(accessClock);
    while (frame.history.size() > static_cast<size_t>(k_param)) {
        frame.history.pop_front();
    }
}

template <typename T>
string LRU_K<T>::pick_victim() const {
    string victim;
    bool found = false;

    bool victimUnderSampled = false;
    size_t victimDistance = 0;
    size_t victimLastAccess = 0;

    for (const auto& [fileName, frame] : cache) {
        bool underSampled = frame.history.size() < static_cast<size_t>(k_param);
        size_t distance = 0;
        if (!underSampled) {
            distance = accessClock - frame.history.front();
        }

        if (!found) {
            victim = fileName;
            victimUnderSampled = underSampled;
            victimDistance = distance;
            victimLastAccess = frame.lastAccess;
            found = true;
            continue;
        }

        if (underSampled && !victimUnderSampled) {
            victim = fileName;
            victimUnderSampled = true;
            victimDistance = distance;
            victimLastAccess = frame.lastAccess;
            continue;
        }

        if (underSampled && victimUnderSampled) {
            if (frame.lastAccess < victimLastAccess) {
                victim = fileName;
                victimDistance = distance;
                victimLastAccess = frame.lastAccess;
            }
            continue;
        }

        if (!underSampled && !victimUnderSampled) {
            if (distance > victimDistance || (distance == victimDistance && frame.lastAccess < victimLastAccess)) {
                victim = fileName;
                victimDistance = distance;
                victimLastAccess = frame.lastAccess;
            }
        }
    }

    return victim;
}

template <typename T>
bool LRU_K<T>::load_from_disk(const string& fileName, Frame& frame, bool* isLeafOut) {
    if constexpr (is_same_v<T, map<int, string>>) {
        ifstream file(fileName, ios::in);
        if (!file.is_open()) {
            return false;
        }

        map<int, string> currNode;
        bool isLeaf = true;
        if (!(file >> isLeaf)) {
            return false;
        }

        string filePtrs;
        if (!isLeaf) {
            if (!(file >> filePtrs)) {
                return false;
            }
            currNode[0] = filePtrs;
        }

        int key;
        while (file >> key >> filePtrs) {
            currNode[key] = filePtrs;
        }

        frame.data = move(currNode);
        frame.isLeaf = isLeaf;
        frame.dirty = false;
        if (isLeafOut != nullptr) {
            *isLeafOut = isLeaf;
        }
        return true;
    } else {
        ifstream file(fileName, ios::in);
        if (!file.is_open()) {
            return false;
        }

        vector<string> lines;
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }

        frame.data = move(lines);
        frame.isLeaf = true;
        frame.dirty = false;
        if (isLeafOut != nullptr) {
            *isLeafOut = true;
        }
        return true;
    }
}

template <typename T>
bool LRU_K<T>::flush_to_disk(const string& fileName, Frame& frame) {
    if (!frame.dirty) {
        return true;
    }

    if constexpr (is_same_v<T, map<int, string>>) {
        ofstream file(fileName, ios::out);
        if (!file.is_open()) {
            return false;
        }

        file << frame.isLeaf << '\n';

        if (!frame.isLeaf) {
            auto ptr = frame.data.find(0);
            if (ptr == frame.data.end()) {
                return false;
            }
            file << ptr->second << '\n';
        }

        for (const auto& [key, filePtr] : frame.data) {
            if (!frame.isLeaf && key == 0) {
                continue;
            }
            file << key << '\n' << filePtr << '\n';
        }

        frame.dirty = false;
        return true;
    } else {
        ofstream file(fileName, ios::out);
        if (!file.is_open()) {
            return false;
        }

        for (size_t i = 0; i < frame.data.size(); i++) {
            file << frame.data[i];
            if (i + 1 < frame.data.size()) {
                file << '\n';
            }
        }

        frame.dirty = false;
        return true;
    }
}

template <typename T>
bool LRU_K<T>::evict_if_needed() {
    lock_guard<mutex> mapLock(cacheMapMutex); map_latches++;
    if (page <= 0 || cache.size() < static_cast<size_t>(page)) return true;

    string victim = pick_victim();
    if (victim.empty()) return true;

    auto it = cache.find(victim);
    if (it == cache.end()) return true;
    
    {
        unique_lock<mutex> frameLock(it->second.frameMutex);
        if (!flush_to_disk(victim, it->second)) return false;
    }
    
    cache.erase(it);
    return true;
}

template <typename T>
void LRU_K<T>::invalidate(const string& fileName) {
    lock_guard<mutex> mapLock(cacheMapMutex);
    auto it = cache.find(fileName);
    if (it != cache.end()) {
        cache.erase(it);
    }
}

template <typename T>
bool LRU_K<T>::read(const string& fileName, T& out, bool* isLeafOut, bool recordStats) {
    {
        lock_guard<mutex> mapLock(cacheMapMutex); map_latches++;
        if (recordStats) ++totalReads;
        auto it = cache.find(fileName);
        if (it != cache.end()) {
            lock_guard<mutex> frameLock(it->second.frameMutex); frame_latches++;
            if (recordStats) ++readHits;
            touch(it->second);
            out = it->second.data;
            if constexpr (is_same_v<T, map<int, string>>) {
                if (isLeafOut != nullptr) {
                    *isLeafOut = it->second.isLeaf;
                }
            } else {
                if (isLeafOut != nullptr) {
                    *isLeafOut = true;
                }
            }
            return true;
        }
    }

    Frame frame;
    if (!load_from_disk(fileName, frame, isLeafOut)) {
        return false;
    }

    if (page <= 0) {
        out = frame.data;
        return true;
    }

    if (!evict_if_needed()) {
        return false;
    }
    lock_guard<mutex> frameLock(frame.frameMutex); frame_latches++;
    touch(frame);
    out = frame.data;
    lock_guard<mutex> mapLock(cacheMapMutex); map_latches++;
    Frame& cached = cache[fileName];
    cached.data = frame.data;
    cached.dirty = frame.dirty;
    cached.isLeaf = frame.isLeaf;
    cached.lastAccess = frame.lastAccess;
    cached.history = frame.history;
    return true;
}

template <typename T>
bool LRU_K<T>::write(const string& fileName, const T& value, bool isLeaf, bool recordStats) {
    if (page <= 0) {
        Frame direct;
        direct.data = value;
        direct.dirty = true;
        if constexpr (is_same_v<T, map<int, string>>) {
            direct.isLeaf = isLeaf;
        } else {
            direct.isLeaf = true;
        }
        return flush_to_disk(fileName, direct);
    }

    {
        lock_guard<mutex> mapLock(cacheMapMutex); map_latches++;
        if (recordStats) ++totalWrites;
        auto it = cache.find(fileName);
        if (it != cache.end()) {
            lock_guard<mutex> frameLock(it->second.frameMutex); frame_latches++;
            if (recordStats) ++writeHits;
            it->second.data = value;
            it->second.dirty = true;
            if constexpr (is_same_v<T, map<int, string>>) {
                it->second.isLeaf = isLeaf;
            } else {
                it->second.isLeaf = true;
            }
            touch(it->second);
            return true;
        }
    }

    if (!evict_if_needed()) {
        return false;
    }
    Frame frame;
    frame.data = value;
    frame.dirty = true;
    if constexpr (is_same_v<T, map<int, string>>) {
        frame.isLeaf = isLeaf;
    } else {
        frame.isLeaf = true;
    }
    lock_guard<mutex> frameLock(frame.frameMutex); frame_latches++;
    touch(frame);
    lock_guard<mutex> mapLock(cacheMapMutex); map_latches++;
    Frame& cached = cache[fileName];
    cached.data = frame.data;
    cached.dirty = frame.dirty;
    cached.isLeaf = frame.isLeaf;
    cached.lastAccess = frame.lastAccess;
    cached.history = frame.history;
    return true;
}

template <typename T>
bool LRU_K<T>::flush(const string& fileName) {
    lock_guard<mutex> mapLock(cacheMapMutex); map_latches++;
    auto it = cache.find(fileName);
    if (it == cache.end()) {
        return true;
    }
    lock_guard<mutex> frameLock(it->second.frameMutex); frame_latches++;
    return flush_to_disk(fileName, it->second);
}

template <typename T>
void LRU_K<T>::flush_all() {
    lock_guard<mutex> mapLock(cacheMapMutex); map_latches++;
    for (auto& [fileName, frame] : cache) {
        lock_guard<mutex> frameLock(frame.frameMutex); frame_latches++;
        flush_to_disk(fileName, frame);
    }
}

template <typename T>
void LRU_K<T>::print_stats() const {
    lock_guard<mutex> lock(cacheMapMutex); map_latches++;
    const char* cacheType = "Unknown Cache";
    if constexpr (is_same_v<T, map<int, string>>) {
        cacheType = "Index Cache";
    } else if constexpr (is_same_v<T, vector<string>>) {
        cacheType = "Data Cache";
    }

    long long totalAccess = totalReads + totalWrites;
    long long totalHits = readHits + writeHits;
    double totalHitRatio = (totalAccess > 0) ? (100.0 * totalHits / totalAccess) : 0.0;
    double readHitRatio = (totalReads > 0) ? (100.0 * readHits / totalReads) : 0.0;
    double writeHitRatio = (totalWrites > 0) ? (100.0 * writeHits / totalWrites) : 0.0;

    cout << "\n===== " << cacheType << " Statistics =====\n";
    cout << "Total Reads:       " << totalReads << "\n";
    cout << "Total Writes:      " << totalWrites << "\n";
    cout << "Read Hits:         " << readHits << "\n";
    cout << "Write Hits:        " << writeHits << "\n";
    cout << "Read Hit Ratio:    " << fixed << setprecision(2) << readHitRatio << "%\n";
    cout << "Write Hit Ratio:   " << fixed << setprecision(2) << writeHitRatio << "%\n";
    cout << "Total Access:      " << totalAccess << "\n";
    cout << "Total Hits:        " << totalHits << "\n";
    cout << "Total Hit Ratio:   " << fixed << setprecision(2) << totalHitRatio << "%\n";
    cout << "Map Latches:       " << map_latches.load() << "\n";
    cout << "Frame Latches:     " << frame_latches.load() << "\n";
    cout << "====================================\n";
}

template class LRU_K<map<int, string>>; // cache for index
template class LRU_K<vector<string>>;   // cache for data rows
