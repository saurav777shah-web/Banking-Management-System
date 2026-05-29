// =============================================================================
// OWNER : Dev 2 — Auth System  (wires everything together)
// FILE  : src/main.cpp
// ABOUT : Entry point. Initialises the DB, runs schema setup, then loops on
//         the login/register screen and routes to the right menu.
//
// TASKS:
//   [ ] Open DB: Database db("banking.db")   ← use the new DB class from Dev 1
//   [ ] Run:     Schema::init(db)             ← creates tables + seeds admin
//   [ ] Loop:
//         === Banking Management System ===
//         1. Login
//         2. Register
//         3. Exit
//         > _
//   [ ] On Login:
//         - prompt username + password
//         - call Auth::loginUser(db, username, password)
//         - on failure: print "Invalid credentials." and loop
//         - on success: check Session::current().role
//             - "admin" → call AdminMenu::run(db)
//             - "user"  → call UserMenu::run(db)
//         - after menu returns (logout), loop back to login screen
//   [ ] On Register:
//         - prompt name, username, password
//         - call Auth::registerUser(db, name, username, password)
//         - print result message, loop back
//   [ ] On Exit: print "Goodbye." and return 0
// =============================================================================
#include "database/db.h"
#include "database/schema.h"
#include "auth/auth.h"
#include "auth/session.h"
#include "user/user_menu.h"
#include "admin/admin_menu.h"

int main() {
    // TODO (Dev 2): implement per the tasks above
    return 0;
}
