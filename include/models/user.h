// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : include/models/user.h
// ABOUT : User struct and all CRUD helpers for the `users` table.
//
// MUST PROVIDE:
//   struct User
//     - int id
//     - std::string name, username, password_hash
//     - std::string role  ("user" | "admin")
//     - std::string created_at
//
//   namespace UserModel
//     - std::optional<User> findById(DB&, int id)
//     - std::optional<User> findByUsername(DB&, const std::string& username)
//     - User                create(DB&, name, username, password_hash, role)
//     - std::vector<User>   findAll(DB&)
//
// DEPENDENCIES: db.h
// =============================================================================
#pragma once
