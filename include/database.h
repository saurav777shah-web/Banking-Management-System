#pragma once
#include <sqlite3.h>
#include <string>
#include <stdexcept>

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void execute(const std::string& sql);
    sqlite3* handle() const { return db_; }

private:
    sqlite3* db_ = nullptr;
};
