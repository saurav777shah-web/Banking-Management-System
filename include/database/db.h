#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <cstdint>

class DB {
public:
    using Row    = std::map<std::string, std::string>;
    using BindFn = std::function<void(sqlite3_stmt*)>;

    explicit DB(const std::string& path);
    ~DB();
    DB(const DB&)            = delete;
    DB& operator=(const DB&) = delete;

    // Run a statement with no result rows (DDL, INSERT, UPDATE, DELETE)
    void execute(const std::string& sql);
    void execute(const std::string& sql, BindFn binder);

    // Run a SELECT and return rows as column-name -> value maps
    std::vector<Row> query(const std::string& sql);
    std::vector<Row> query(const std::string& sql, BindFn binder);

    int64_t  lastInsertId() const;
    sqlite3* handle()       const { return db_; }

private:
    sqlite3*         db_ = nullptr;
    std::vector<Row> runQuery(const std::string& sql, BindFn binder);
};
