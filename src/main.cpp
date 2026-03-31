#include "../include/bptree.hpp"
#include <signal.h>

priority_queue<int> BPlusTree::availPointer;
static LRU_K<vector<string>>* g_dataCache = nullptr;

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
        "name=user_" + to_string(key),
        "dept=dept_" + to_string(key % 69),
        "score=" + to_string((key * 11) % 69)
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
    iota(keys.begin(), keys.end(), 1);
    random_device rd;
    mt19937 rng(rd());
    shuffle(keys.begin(), keys.end(), rng);
    for (int key : keys) {
        if (!tree.Insert(key)) {
            cout << "Insert failed for key=" << key << endl;
            print_failure_from_bp_error();
            return false;
        }
        string dataFile = tree.Search(key);
        if (dataFile.empty()) {
            cout << "Post-insert search failed for key=" << key << endl;
            return false;
        }
        if (!write_row_file(dataFile, key, deterministic_fields(key))) {
            return false;
        }
    }
    cout << "Insert phase complete."<<endl;
    return true;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(&cout);

    signal(SIGINT, handle_interrupt);

    ensure_storage_dirs();
    g_tree = new BPlusTree();
    BPlusTree& tree = *g_tree;
    while (true) {
        cout << "\n===== B+ Tree Menu =====\n";
        cout << "1. Insert"<<endl;
        cout << "2. Search"<<endl;
        cout << "3. Delete"<<endl;
        cout << "4. Add a number of rows"<<endl;
        cout << "5. Exit"<<endl;
        cout << "Enter choice: ";
        int choice;
        cin>>choice;
        while(choice < 1 || choice > 5){
            cout << "Invalid choice. Enter a number between 1 and 5: ";
            cin >> choice;
        }
        if (choice == 1) {
            int key;
            cout << "Enter key (>0): ";
            cin >> key;
            if(!cin || key <= 0){
                cout << "Invalid key. Please enter a positive integer."<<endl;
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
            if(!cin || key <= 0){
                cout << "Invalid key. Please enter a positive integer."<<endl;
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
            if(!cin || key <= 0){
                cout << "Invalid key. Please enter a positive integer."<<endl;
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