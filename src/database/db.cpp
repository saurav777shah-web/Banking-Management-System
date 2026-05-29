#include "database/db.h"

DB::DB(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Cannot open database: " + err);
    }
    execute("PRAGMA foreign_keys = ON;");
    execute("PRAGMA journal_mode = WAL;");
}

DB::~DB() {
    if (db_) sqlite3_close(db_);
}

void DB::execute(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err;
        sqlite3_free(err);
        throw std::runtime_error("SQL error: " + msg);
    }
}

void DB::execute(const std::string& sql, BindFn binder) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(std::string("Prepare error: ") + sqlite3_errmsg(db_));

    if (binder) binder(stmt);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        throw std::runtime_error(std::string("Execute error: ") + sqlite3_errmsg(db_));
}

std::vector<DB::Row> DB::query(const std::string& sql) {
    return runQuery(sql, nullptr);
}

std::vector<DB::Row> DB::query(const std::string& sql, BindFn binder) {
    return runQuery(sql, std::move(binder));
}

std::vector<DB::Row> DB::runQuery(const std::string& sql, BindFn binder) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(std::string("Query error: ") + sqlite3_errmsg(db_));

    if (binder) binder(stmt);

    std::vector<Row> rows;
    int cols = sqlite3_column_count(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;
        for (int i = 0; i < cols; ++i) {
            const char* name = sqlite3_column_name(stmt, i);
            const char* val  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[name ? name : ""] = val ? val : "";
        }
        rows.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    return rows;
}

int64_t DB::lastInsertId() const {
    return sqlite3_last_insert_rowid(db_);
}
