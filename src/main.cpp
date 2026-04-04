#include "../include/bptree.hpp"
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>

int K;
int INDEX_PAGES;
int DATA_PAGES;

static vector<string> parse_csv_line(const string& line, char delimiter){
    vector<string> out;
    string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field.push_back('"');
                    i++;
                } else {
                    inQuotes = false;
                }
            } else {
                field.push_back(c);
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == delimiter) {
                out.push_back(field);
                field.clear();
            } else {
                field.push_back(c);
            }
        }
    }
    out.push_back(field);
    return out;
}


priority_queue<int, vector<int>, greater<int>>BPlusTree::availPointer;
static LRU_K<vector<string>>* g_dataCache = nullptr;

static constexpr int MAX_KEY_VALUE = 200000;

static BPlusTree* g_tree = nullptr;

static void destroy_tree(){
    if (g_tree != nullptr) {
        delete g_tree;
        g_tree = nullptr;
    }
}

static void destroy_data_cache(){
    if (g_dataCache != nullptr) {
        delete g_dataCache;
        g_dataCache = nullptr;
    }
}

static void handle_interrupt(int signum){
    destroy_tree();
    destroy_data_cache();
    exit(0);
}

void ensure_storage_dirs(){
    filesystem::create_directories("Data");
    filesystem::create_directories("Index");
}

bool write_row_file(const string& filePath, int key, const vector<string>& fields){
    // ofstream out(filePath, ios::out);
    // if (!out.is_open()) {
    //     cerr << "(ERROR) Failed to write row file: " << filePath << endl;
    //     return false;
    // }
    // for (size_t i = 0; i < fields.size(); i++) {
    //     out << fields[i];
    //     if (i + 1 < fields.size()) {
    //         out << '\n';
    //     }
    // }
    // return true;
    if (g_dataCache == nullptr) return false;
    return g_dataCache->write(filePath, fields, false);
}

static bool load_dataset_into_tree(BPlusTree& tree, const string& csvPath){
    ifstream in(csvPath);
    if (!in.is_open()) {
        cout << "(ERROR) Could not open dataset: " << csvPath << endl;
        return false;
    }

    constexpr char CSV_DELIM = ';';

    string headerLine;
    if (!getline(in, headerLine)) {
        cout << "(ERROR) Dataset is empty: " << csvPath << endl;
        return false;
    }
    if (!headerLine.empty() && headerLine.back() == '\r') headerLine.pop_back();
    vector<string> headers = parse_csv_line(headerLine, CSV_DELIM);
    if (headers.empty()) {
        cout << "(ERROR) Failed to parse CSV header." << endl;
        return false;
    }

    cout << "Loading dataset into B+Tree: " << csvPath << endl;

    long long inserted = 0;
    long long skipped = 0;
    long long lineNo = 1; // header is line 1

    int key = 1;
    string record;
    while (getline(in, record)) {
        if (!record.empty() && record.back() == '\r') record.pop_back();
        lineNo++;
        if (record.empty()) continue;

        if (key > MAX_KEY_VALUE) {
            cout << "Reached max key value at input line " << lineNo << ". Stopping." << endl;
            break;
        }

        vector<string> values = parse_csv_line(record, CSV_DELIM);
        if (values.size() < headers.size()) {
            values.resize(headers.size());
        }

        // Find an unused key (handles case where user already inserted some keys)
        bool insertedThisRow = false;
        while (key <= MAX_KEY_VALUE) {
            if (tree.Insert(key)) {
                string dataFile = tree.Search(key);
                if (dataFile.empty()) {
                    tree.Delete(key);
                    skipped++;
                    key++;
                    break;
                }

                // Store only values (one attribute per line), to save space.
                vector<string> rowLines;
                rowLines.reserve(headers.size());
                for (size_t i = 0; i < headers.size(); i++) {
                    string v = values[i];
                    for (char& ch : v) {
                        if (ch == '\n' || ch == '\r') ch = ' ';
                    }
                    rowLines.push_back(v);
                }

                if (!write_row_file(dataFile, key, rowLines)) {
                    tree.Delete(key);
                    skipped++;
                    key++;
                    break;
                }

                inserted++;
                key++;
                insertedThisRow = true;
                break;
            }
            key++;
        }

        if (!insertedThisRow && key > MAX_KEY_VALUE) {
            cout << "Reached max key value at input line " << lineNo << ". Stopping." << endl;
            break;
        }

        if (inserted > 0 && (inserted % 5000 == 0)) {
            cout << "Loaded " << inserted << " rows..." << endl;
        }
    }

    cout << "Dataset load complete. Inserted=" << inserted << ", skipped=" << skipped << endl;
    return inserted > 0;
}

vector<string> read_row_file_lines(const string& filePath){
    // ifstream in(filePath, ios::in);
    // vector<string> lines;
    // if (!in.is_open()) {
    //     return lines;
    // }
    // string line;
    // while (getline(in, line)) {
    //     lines.push_back(line);
    // }
    // return lines;
    if (g_dataCache == nullptr) return {};
    vector<string> currData;
    g_dataCache->read(filePath, currData, nullptr);
    return currData;
}


vector<string> deterministic_fields(int key){
    return {
        "user_" + to_string(key),
        "dept_" + to_string(key % 69),
        to_string((key * 11) % 69)
    };
}

void print_failure_from_bp_error(){
    auto it = g_ErrorMsg.find(BP_ERROR);
    if (it != g_ErrorMsg.end()) {
        cout << it->second << endl;
    } else {
        cout << "Operation failed with unknown error."<<endl;
    }
}

bool add_multiple_rows(BPlusTree& tree, int totalRows){
    cout << "Inserting " << totalRows << " rows..."<<endl;
    vector<int> keys(totalRows);
    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> dist(1, MAX_KEY_VALUE);
    for (int i = 0; i < totalRows; i++) {
        keys[i] = dist(rng);
    }
    
    for (int key : keys) {
        if (!tree.Insert(key)) {
            cout << "Insert failed for key=" << key << endl;
            print_failure_from_bp_error();
            // return false;
            continue;
        }
        string dataFile = tree.Search(key);
        if (dataFile.empty()) {
            cout << "Post-insert search failed for key=" << key << endl;
            // return false;
            continue;
        }
        if (!write_row_file(dataFile, key, deterministic_fields(key))) {
            cout<< "Write row failed for key=" << key << endl;
            // return false;
            continue;
        }
    }
    cout << "Insert phase complete."<<endl;
    return true;
}

void run_concurrent_stress(BPlusTree& tree, int numThreads, int opsPerThread, int insPct=45, int srchPct=35, bool csvOut=false){
    if (numThreads <= 0 || opsPerThread <= 0) {
        cout << "Thread count and operations must be positive." << endl;
        return;
    }

    atomic<long long> insertOk(0), insertFail(0);
    atomic<long long> deleteOk(0), deleteFail(0);
    atomic<long long> searchHit(0), searchMiss(0);
    atomic<long long> timeInsNs(0), timeSrchNs(0), timeDelNs(0);

    auto worker = [&](int tid){
        mt19937 rng(static_cast<unsigned>(chrono::steady_clock::now().time_since_epoch().count()) ^ static_cast<unsigned>(tid * 7919));
        uniform_int_distribution<int> opDist(0, 99);
        uniform_int_distribution<int> keyDist(1, MAX_KEY_VALUE);

        for (int i = 0; i < opsPerThread; i++) {
            int key = keyDist(rng);
            int op = opDist(rng);

            auto op_start = chrono::steady_clock::now();
            if (op < insPct) {
                if (tree.Insert(key)) {
                    insertOk++;
                    string dataFile = tree.Search(key);
                    if (!dataFile.empty()) {
                        write_row_file(dataFile, key, deterministic_fields(key));
                    }
                } else {
                    insertFail++;
                }
                auto op_end = chrono::steady_clock::now();
                timeInsNs += chrono::duration_cast<chrono::nanoseconds>(op_end - op_start).count();
            } else if (op < insPct + srchPct) {
                string dataFile = tree.Search(key);
                if (dataFile.empty()) {
                    searchMiss++;
                } else {
                    searchHit++;
                }
                auto op_end = chrono::steady_clock::now();
                timeSrchNs += chrono::duration_cast<chrono::nanoseconds>(op_end - op_start).count();
            } else {
                if (tree.Delete(key)) {
                    deleteOk++;
                } else {
                    deleteFail++;
                }
                auto op_end = chrono::steady_clock::now();
                timeDelNs += chrono::duration_cast<chrono::nanoseconds>(op_end - op_start).count();
            }
        }
    };

    if (!csvOut) {
        cout << "Starting concurrent stress: threads=" << numThreads
             << ", ops/thread=" << opsPerThread << "..." << endl;
    }

    auto start = chrono::steady_clock::now();
    vector<thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(worker, i + 1);
    }
    for (auto& t : threads) {
        t.join();
    }
    auto end = chrono::steady_clock::now();

    long long elapsedMs = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    if (csvOut) {
        long long totalIns = insertOk + insertFail;
        long long totalSrch = searchHit + searchMiss;
        long long totalDel = deleteOk + deleteFail;
        
        double avgInsMs = totalIns > 0 ? (double)timeInsNs / totalIns / 1e6 : 0.0;
        double avgSrchMs = totalSrch > 0 ? (double)timeSrchNs / totalSrch / 1e6 : 0.0;
        double avgDelMs = totalDel > 0 ? (double)timeDelNs / totalDel / 1e6 : 0.0;
        
        int delPct = 100 - insPct - srchPct;
        cout << K << "," << INDEX_PAGES << "," << DATA_PAGES << "," << numThreads << "," << (numThreads * opsPerThread) 
             << "," << insPct << "," << srchPct << "," << delPct << "," << elapsedMs << "," 
             << insertOk << "," << insertFail << "," << deleteOk << "," << deleteFail << "," 
             << searchHit << "," << searchMiss << "," << avgInsMs << "," << avgSrchMs << "," << avgDelMs << "\n";
        return;
    }

    cout << "Concurrent stress complete." << endl;
    cout << "Elapsed: " << elapsedMs << " ms" << endl;
    cout << "Insert  ok/fail: " << insertOk << "/" << insertFail << endl;
    cout << "Delete  ok/fail: " << deleteOk << "/" << deleteFail << endl;
    cout << "Search  hit/miss: " << searchHit << "/" << searchMiss << endl;
}

int main(int argc, char* argv[]){
    ios::sync_with_stdio(false);
    cin.tie(&cout);

    if (argc > 1 && string(argv[1]) == "--experiment") {
        if (argc >= 9) {
            K = stoi(argv[2]);
            INDEX_PAGES = stoi(argv[3]);
            DATA_PAGES = stoi(argv[4]);
            int numThreads = stoi(argv[5]);
            int opsPerThread = stoi(argv[6]);
            int insPct = stoi(argv[7]);
            int searchPct = stoi(argv[8]);

            g_dataCache = new LRU_K<vector<string>>(K, DATA_PAGES);
            ensure_storage_dirs();
            g_tree = new BPlusTree();
            
            run_concurrent_stress(*g_tree, numThreads, opsPerThread, insPct, searchPct, true);
            
            destroy_tree();
            destroy_data_cache();
            return 0;
        } else {
            cout << "Usage for expt: ./bptree --experiment <K> <INDEX_PAGES> <DATA_PAGES> <THREADS> <OPS_PER_THREAD> <INS_PCT> <SRCH_PCT>\n";
            return 1;
        }
    }

    // Default values back for interactive mode if not run via experiment script
    K = 2;
    INDEX_PAGES = 100;
    DATA_PAGES = 1000;

    signal(SIGINT, handle_interrupt);

    g_dataCache = new LRU_K<vector<string>>(K, DATA_PAGES);

    ensure_storage_dirs();
    g_tree = new BPlusTree();
    BPlusTree& tree = *g_tree;
    while (true) {
        cout << "\n===== B+ Tree Menu =====\n";
        cout << "1. Insert"<<endl;
        cout << "2. Search"<<endl;
        cout << "3. Delete"<<endl;
        cout << "4. Add a number of rows"<<endl;
        cout << "5. Concurrent stress test"<<endl;
        cout << "6. Load a dataset"<<endl;
        cout << "7. Exit"<<endl;
        cout << "Enter choice: ";
        int choice;
        cin>>choice;
        while(choice < 1 || choice > 7){
            cout << "Invalid choice. Enter a number between 1 and 7: ";
            cin >> choice;
        }
        if (choice == 1) {
            int key;
            cout << "Enter key (1-" << MAX_KEY_VALUE << "): ";
            cin >> key;
            if(!cin || key <= 0 || key > MAX_KEY_VALUE){
                cout << "Invalid key. Please enter a value between 1 and " << MAX_KEY_VALUE << "."<<endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            auto fields = deterministic_fields(key);
            if (!tree.Insert(key)) {
                print_failure_from_bp_error();
                continue;
            }
            string dataFile = tree.Search(key);
            if (!write_row_file(dataFile, key, fields)) {
                cout << "Insertion failed: could not write row file."<<endl;
                continue;
            }
            cout << "Insertion successful. Row file: " << dataFile << endl;
        } else if (choice == 2) {
            int key;
            cout << "Enter key to search: ";
            cin >> key;
            if(!cin || key <= 0 || key > MAX_KEY_VALUE){
                cout << "Invalid key. Please enter a value between 1 and " << MAX_KEY_VALUE << "."<<endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            string dataFile = tree.Search(key);
            if (dataFile.empty()) {
                cout << "Key not found.\n";
            } else {
                cout << "Key found. Row file: "<< dataFile << endl;
                auto lines = read_row_file_lines(dataFile);
                if (lines.empty()) {
                    cout << "Row file is empty or unreadable.\n";
                } else {
                    cout << "Row data:\n";
                    for (const auto& line : lines) cout << "  " << line << endl;
                }
            }
        } else if (choice == 3) {
            int key;
            cout << "Enter key to delete: ";
            cin >> key;
            if(!cin || key <= 0 || key > MAX_KEY_VALUE){
                cout << "Invalid key. Please enter a value between 1 and " << MAX_KEY_VALUE << "."<<endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
                }
            if (!tree.Delete(key)) {
                print_failure_from_bp_error();
                continue;
            }
            cout << "Delete successful for key=" << key << endl;
        } else if (choice == 4) {
            int tmp;
            cout << "Enter number of rows to add: ";
            cin >> tmp;
            add_multiple_rows(tree, tmp);
        } else if (choice == 5) {
            int numThreads = 0;
            int opsPerThread = 0;
            cout << "Enter number of threads: ";
            cin >> numThreads;
            cout << "Enter operations per thread: ";
            cin >> opsPerThread;
            if (!cin) {
                cout << "Invalid input." << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            run_concurrent_stress(tree, numThreads, opsPerThread);
        } else if (choice == 6) {
            load_dataset_into_tree(tree, "apartments_for_rent_classified_100K.csv");
        } else if (choice == 7) {
            cout << "Exiting bitches bbye!!!."<<endl;
            break;
        } else {
            cout << "Invalid choice. Try again."<<endl;
        }
    }
    destroy_tree();
    destroy_data_cache();
    return 0;
}