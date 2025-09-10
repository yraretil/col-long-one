#include<iostream>
#include<vector>
#include<string>
#include<ctime>
#include<sstream>

using namespace std;

struct TreeNode {
    int version_id;
    string content;
    string message;
    time_t created_timestamp;
    time_t snapshot_timestamp;
    TreeNode* parent;
    vector<TreeNode*> children;
    bool is_snapshot;

    TreeNode(int id, const string& con = "", TreeNode* p = nullptr) : version_id(id), content(con), parent(p), is_snapshot(false) {
        created_timestamp = time(nullptr);
        snapshot_timestamp = 0;
    }
};


class HashMap {
private:
    static const int INIT_CAPACITY = 8;
    vector<vector<pair<string, File*>>> table;
    int numElements;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    int hash(const string& key) const {
        unsigned long h = 5381;
        for (char c : key) h = ((h << 5) + h) + c;
        return h % table.size();
    }

    void rehash() {
        int newCapacity = table.size() * 2;
        vector<vector<pair<string, File*>>> newTable(newCapacity);

        for (auto& bucket : table) {
            for (auto& p : bucket) {
                int idx = 0;
                unsigned long h = 5381;
                for (char c : p.first) h = ((h << 5) + h) + c;
                idx = h % newCapacity;
                newTable[idx].push_back(p);
            }
        }
        table.swap(newTable);
    }

public:
    HashMap() : table(INIT_CAPACITY), numElements(0) {}

    bool insert(const string& key, File* value) {
        int idx = hash(key);
        for (auto& p : table[idx]) {
            if (p.first == key) return false;
        }
        table[idx].push_back({key, value});
        numElements++;

        if ((double)numElements / table.size() > MAX_LOAD_FACTOR) {
            rehash();
        }
        return true;
    }

    File* find(const string& key) {
        int idx = hash(key);
        for (auto& p : table[idx]) {
            if (p.first == key) return p.second;
        }
        return nullptr;
    }
};



struct File {
    TreeNode* root;
    TreeNode* active_version;
    vector<TreeNode*> version_map;
    int total_versions;
    string filename;
    time_t last_modified;

    File(const string& name) : filename(name), total_versions(1) {
        root = new TreeNode(0);
        active_version = root;
        version_map.push_back(root);
        last_modified = time(nullptr);
    }
};


class System {
    private:
        HashMap files;

    public:
        System() {}

        void create(const string& filename) {
            if (files.find(filename)) {
                cout << "Error: File '" << filename << "' already exists." << endl;
                return;
            }

            File* new_file = new File(filename);
            files.insert(filename, new_file);
            cout << "File '" << filename << "' created successfully." << endl;
        }

        void read(const string& filename) {
            File* file = files.find(filename);
            if (!file) {
                cout << "Error: File '" << filename << "' not found." << endl;
                return;
            }

            cout << file->active_version->content << endl;
        }

        void insert(const string& filename, const string& content) {
            File* file = files.find(filename);
            if (!file) {
                cout << "Error: File '" << filename << "' not found." << endl;
                return;
            }

            if (file->active_version->is_snapshot) {
                TreeNode* new_version = new TreeNode(file->total_versions, file->active_version->content + content, file->active_version);
                file->active_version->children.push_back(new_version);
                file->version_map.push_back(new_version);
                file->active_version = new_version;
                file->total_versions++;
            } 
            else file->active_version->content += content;

            file->last_modified = time(nullptr);
            cout << "Content inserted successfully." << endl;
        }
        
        void update(const string& filename, const string& content) {
            File* file = files.find(filename);
            if (!file) {
                cout << "Error: File '" << filename << "' not found." << endl;
                return;
            }

            if (file->active_version->is_snapshot) {
                TreeNode* new_version = new TreeNode(file->total_versions, content, file->active_version);
                file->active_version->children.push_back(new_version);
                file->version_map.push_back(new_version);
                file->active_version = new_version;
                file->total_versions++;
            } 
            else file->active_version->content = content;

            file->last_modified = time(nullptr);
            cout << "Content updated successfully." << endl;
        }

        void snapshot(const string& filename, const string& message) {
            File* file = files.find(filename);
            if (!file) {
                cout << "Error: File '" << filename << "' not found." << endl;
                return;
            }

            if (file->active_version->is_snapshot) {
                cout << "Error: Current version is already a snapshot." << endl;
                return;
            }

            file->active_version->message = message;
            file->active_version->is_snapshot = true;
            file->active_version->snapshot_timestamp = time(nullptr);
            file->last_modified = time(nullptr);

            cout << "Snapshot created with message: '" << message << "'" << endl;
        }

        void rollback(const string& filename, int version_id = -1) {
            File* file = files.find(filename);
            if (!file) {
                cout << "Error: File '" << filename << "' not found." << endl;
                return;
            }

            if (version_id == -1) {
                if (!file->active_version->parent) {
                    cout << "Error: No parent version to rollback to." << endl;
                    return;
                }
                file->active_version = file->active_version->parent;
                cout << "Rolled back to parent version." << endl;
                return;
            }

            if (version_id < 0 || version_id >= file->total_versions) {
                cout << "Error: Invalid version ID." << endl;
                return;
            }

            TreeNode* target = nullptr;
            for (TreeNode* v : file->version_map) {
                if (v->version_id == version_id) {
                    target = v;
                    break;
                }
            }

            if (!target) {
                cout << "Error: Version " << version_id << " not found." << endl;
                return;
            }

            file->active_version = target;
            cout << "Rolled back to version " << version_id << endl;
        }

        void history(const string& filename) {
            File* file = files.find(filename);
            if (!file) {
                cout << "Error: File '" << filename << "' not found." << endl;
                return;
            }

            cout << "Version history for file: " << filename << endl;
            for (TreeNode* v : file->version_map) {
                if(v->is_snapshot) cout << "Version -" << v->version_id << " (Snapshot) - " << ctime(&v->snapshot_timestamp) << "Message: " << v->message << endl;
            }
        }

};

int main() {
    System H;
    cout << "Time-Travelling File System Engaged!" << endl << "Enter commands:" << endl;
    string line;
    while(getline(cin, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        string command;
        ss >> command;

        try{
            if(command == "CREATE") {
                string filename;
                ss >> filename;
                H.create(filename);
            } 
            else if(command == "READ") {
                string filename;
                ss >> filename;
                H.read(filename);
            } 
            else if(command == "INSERT") {
                string filename, content;
                ss >> filename;
                getline(ss, content);
                if(!content.empty() && content[0] == ' ') content = content.substr(1);
                H.insert(filename, content);
            } 
            else if(command == "UPDATE") {
                string filename, content;
                ss >> filename;
                getline(ss, content);
                if(!content.empty() && content[0] == ' ') content = content.substr(1);
                H.update(filename, content);
            } 
            else if(command == "SNAPSHOT") {
                string filename, message;
                ss >> filename;
                getline(ss, message);
                if(!message.empty() && message[0] == ' ') message = message.substr(1);
                H.snapshot(filename, message);
            } 
            else if(command == "ROLLBACK") {
                string filename;
                int version_id = -1;
                ss >> filename;

                if(ss >> version_id) {
                    H.rollback(filename, version_id);
                    cout << "Rolling back file: " << filename << " to version: " << version_id << endl;
                } 
                else H.rollback(filename);
            }
            else cout << "Unknown command: " << command << endl;
        } catch(const exception& e) {
            cout << "Error processing command: " << e.what() << endl;
        }
    }
}