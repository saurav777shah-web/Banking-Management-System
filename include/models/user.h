#pragma once
#include <string>
#include <optional>
#include <vector>
#include "database/db.h"

struct User {
    int         id            = 0;
    std::string name;
    std::string username;
    std::string password_hash;
    std::string role;          // "user" | "admin"
    std::string created_at;
};

namespace UserModel {
    std::optional<User> findById      (DB& db, int id);
    std::optional<User> findByUsername(DB& db, const std::string& username);
    User                create        (DB& db, const std::string& name,
                                               const std::string& username,
                                               const std::string& password_hash,
                                               const std::string& role);
    std::vector<User>   findAll       (DB& db);
}
