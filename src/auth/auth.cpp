// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : src/auth/auth.cpp
// ABOUT : Implement Auth::registerUser() and Auth::loginUser().
//
// TASKS:
//   [ ] registerUser(db, name, username, password)
//         → check username not already taken (UserModel::findByUsername)
//         → hash the password (Password::hash)
//         → create user with role "user" (UserModel::create)
//         → create account for that user (AccountModel::create)
//         → print "Registration successful. Account: ACC000XXX"
//         → return false if username already exists, true on success
//
//   [ ] loginUser(db, username, password)
//         → fetch user by username (UserModel::findByUsername)
//         → verify password (Password::verify)
//         → if admin: SessionData{id, username, "admin", account_id=0}
//         → if user:  fetch account, SessionData{id, username, "user", account_id}
//         → call Session::start()
//         → return false on bad credentials, true on success
// =============================================================================
#include "auth/auth.h"
