// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : include/database/db.h
// ABOUT : Core database connection wrapper around sqlite3.
//         All other modules use this class to talk to the DB.
//         No other file should include sqlite3.h directly.
//
// MUST PROVIDE:
//   class DB
//     - DB(const std::string& path)  → opens/creates the .db file
//     - ~DB()                        → closes connection
//     - execute(sql)                 → run a statement with no result rows
//     - query(sql, bind_fn)          → run SELECT, return rows as vector of maps
//     - sqlite3* handle()            → raw handle for prepared statements
//
// DEPENDENCIES: sqlite3.h (from lib/sqlite3/)
// =============================================================================
#pragma once
