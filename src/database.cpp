#include "database.h"

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error(std::string("Cannot open database: ") + sqlite3_errmsg(db_));
    }
    execute("PRAGMA foreign_keys = ON;");
    execute("PRAGMA journal_mode = WAL;");
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::execute(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err;
        sqlite3_free(err);
        throw std::runtime_error("SQL error: " + msg);
    }
}
