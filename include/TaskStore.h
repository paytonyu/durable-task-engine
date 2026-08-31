

class TaskStore {
private:
    sqlite3* db = nullptr;
    mutable std::mutex mtx;

public:
    explicit TaskStore(const std::string& path) {  // 
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("failed to open database");
        }

        const char* schema = R"(
            PRAGMA journal_mode=WAL;
            PRAGMA synchronous=NORMAL;

            CREATE TABLE IF NOT EXISTS tasks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                payload TEXT NOT NULL,
                status TEXT NOT NULL,
                attempts INTEGER NOT NULL DEFAULT 0,
                created_at INTEGER NOT NULL,
                updated_at INTEGER NOT NULL
            );

            CREATE TABLE IF NOT EXISTS results (
                task_id INTEGER PRIMARY KEY,
                output TEXT
            );
        )";

        char* err = nullptr;
        if (sqlite3_exec(db, schema, nullptr, nullptr, &err) != SQLITE_OK) {
            std::string msg = err ? err : "unknown error";
            sqlite3_free(err);
            sqlite3_close(db);
            throw std::runtime_error("schema init failed: " + msg);
        }
    }

    ~TaskStore() {
        if (db) sqlite3_close(db);
    }

    TaskStore(const TaskStore&) = delete;
    TaskStore& operator=(const TaskStore&) = delete;
};