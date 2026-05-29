// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : src/models/user.cpp
// ABOUT : Implement all UserModel functions declared in include/models/user.h.
//
// TASKS:
//   [ ] findById(db, id)
//         → SELECT * FROM users WHERE id = ?
//         → return std::optional<User> (nullopt if not found)
//   [ ] findByUsername(db, username)
//         → SELECT * FROM users WHERE username = ?
//         → return std::optional<User>
//   [ ] create(db, name, username, password_hash, role)
//         → INSERT INTO users ...
//         → return the newly created User (with id from last_insert_rowid)
//   [ ] findAll(db)
//         → SELECT * FROM users ORDER BY id
//         → return std::vector<User>
// =============================================================================
#include "models/user.h"
