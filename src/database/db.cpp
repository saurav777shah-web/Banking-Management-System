// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : src/database/db.cpp
// ABOUT : Implement the DB class declared in include/database/db.h.
//
// TASKS:
//   [ ] Constructor: call sqlite3_open(); throw on failure
//   [ ] Destructor:  call sqlite3_close()
//   [ ] execute():   sqlite3_exec() for DDL/DML with no result rows
//   [ ] query():     sqlite3_prepare_v2 + step loop, return rows as
//                    std::vector<std::map<std::string, std::string>>
//   [ ] Enable WAL mode and foreign keys in constructor
//
// NOTE: This replaces the old src/database.cpp. Delete that file once done.
// =============================================================================
#include "database/db.h"
