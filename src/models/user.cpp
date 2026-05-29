#include "models/user.h"

static User rowToUser(const DB::Row& r) {
    User u;
    u.id            = std::stoi(r.at("id"));
    u.name          = r.at("name");
    u.username      = r.at("username");
    u.password_hash = r.at("password_hash");
    u.role          = r.at("role");
    u.created_at    = r.at("created_at");
    return u;
}

namespace UserModel {

std::optional<User> findById(DB& db, int id) {
    auto rows = db.query(
        "SELECT * FROM users WHERE id = ? LIMIT 1;",
        [id](sqlite3_stmt* s) { sqlite3_bind_int(s, 1, id); }
    );
    if (rows.empty()) return std::nullopt;
    return rowToUser(rows[0]);
}

std::optional<User> findByUsername(DB& db, const std::string& username) {
    auto rows = db.query(
        "SELECT * FROM users WHERE username = ? LIMIT 1;",
        [&](sqlite3_stmt* s) {
            sqlite3_bind_text(s, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        }
    );
    if (rows.empty()) return std::nullopt;
    return rowToUser(rows[0]);
}

User create(DB& db, const std::string& name, const std::string& username,
            const std::string& password_hash, const std::string& role) {
    db.execute(
        "INSERT INTO users (name, username, password_hash, role) VALUES (?, ?, ?, ?);",
        [&](sqlite3_stmt* s) {
            sqlite3_bind_text(s, 1, name.c_str(),          -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, username.c_str(),       -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 3, password_hash.c_str(),  -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 4, role.c_str(),           -1, SQLITE_TRANSIENT);
        }
    );
    return *findById(db, static_cast<int>(db.lastInsertId()));
}

std::vector<User> findAll(DB& db) {
    auto rows = db.query("SELECT * FROM users ORDER BY id;");
    std::vector<User> users;
    users.reserve(rows.size());
    for (const auto& r : rows) users.push_back(rowToUser(r));
    return users;
}

} // namespace UserModel
