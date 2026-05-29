# Dev 1 — Update Log
**Role:** Database & Models
**Date:** 2026-05-29
**Status:** Complete

---

## What Was Implemented

### 1. `DB` Class — Core Database Wrapper
**Files:** `include/database/db.h` · `src/database/db.cpp`

A single class that wraps the raw `sqlite3*` handle and exposes four clean methods to the rest of the codebase:

| Method | Purpose |
|--------|---------|
| `execute(sql)` | Run DDL / DML with no result rows (CREATE, INSERT, UPDATE) |
| `execute(sql, binder)` | Same but with parameter binding via a lambda |
| `query(sql)` | Run a SELECT, returns rows as `vector<map<string,string>>` |
| `query(sql, binder)` | Same with parameter binding |
| `lastInsertId()` | Returns the row id of the last INSERT |
| `handle()` | Exposes the raw `sqlite3*` for edge cases (used in schema seed) |

**Design decision — why a wrapper instead of raw sqlite3 everywhere?**
Every other module (models, schema, auth) would otherwise need to `#include <sqlite3.h>` directly and repeat the same prepare/step/finalize boilerplate. Wrapping it in one class means:
- Only `db.h` ever touches sqlite3 internals.
- All other files just call `db.execute(...)` or `db.query(...)`.
- Errors are thrown as `std::runtime_error`, so callers don't manage sqlite return codes.

**Design decision — binder lambda pattern:**
SQL parameters are bound via a `std::function<void(sqlite3_stmt*)>` callback instead of string interpolation. This prevents SQL injection and keeps the query strings clean and readable.

---

### 2. `Schema::init` — Table Creation + Admin Seed
**Files:** `include/database/schema.h` · `src/database/schema.cpp`

Called once at startup from `main.cpp`. Does two things:

**A) Creates all three tables using `IF NOT EXISTS`:**
- `users` — stores both regular users and admins, role is enforced with a `CHECK` constraint
- `accounts` — one account per user, balance enforced `>= 0` via `CHECK`
- `transactions` — every money transfer with status `pending / approved / rejected`, enforced with `CHECK`

Using `IF NOT EXISTS` means this function is safe to call on every startup — it only creates tables the first time; subsequent runs are no-ops.

**B) Seeds the default admin account** if no admin exists yet:
```
username : admin
password : admin123
role     : admin
```
The password is stored hashed (using `Password::hash()`), never plain text. The check `WHERE role = 'admin' LIMIT 1` ensures the seed only runs on a fresh database.

---

### 3. `UserModel` — User CRUD
**Files:** `include/models/user.h` · `src/models/user.cpp`

The `User` struct mirrors the `users` table exactly:
```cpp
struct User { int id; string name, username, password_hash, role, created_at; };
```

Four functions in the `UserModel` namespace:

| Function | SQL |
|----------|-----|
| `findById` | `SELECT * FROM users WHERE id = ?` |
| `findByUsername` | `SELECT * FROM users WHERE username = ?` |
| `create` | `INSERT INTO users ...` then re-fetch by last insert id |
| `findAll` | `SELECT * FROM users ORDER BY id` |

Return types use `std::optional<User>` for lookups (returns `nullopt` if not found) and `std::vector<User>` for list queries. This forces callers to handle the not-found case explicitly.

---

### 4. `AccountModel` — Account CRUD
**Files:** `include/models/account.h` · `src/models/account.cpp`

The `Account` struct mirrors the `accounts` table:
```cpp
struct Account { int id, user_id; string account_number; double balance; string created_at; };
```

Five functions:

| Function | SQL |
|----------|-----|
| `create` | Generates account number, `INSERT INTO accounts ...` |
| `findByUserId` | `SELECT * FROM accounts WHERE user_id = ?` |
| `findByAccountNumber` | `SELECT * FROM accounts WHERE account_number = ?` |
| `updateBalance` | `UPDATE accounts SET balance = ? WHERE id = ?` |
| `findAll` | `SELECT * FROM accounts ORDER BY id` |

**Account number generation:**
Account numbers follow the format `ACC000001`, `ACC000002`, etc. — generated from the `user_id` using `snprintf(buf, sizeof(buf), "ACC%06d", user_id)`. This is deterministic (same user always gets the same number) and readable.

---

### 5. `TransactionModel` — Transaction CRUD
**Files:** `include/models/transaction.h` · `src/models/transaction.cpp`

The `Transaction` struct mirrors the `transactions` table:
```cpp
struct Transaction {
    int id, from_account_id, to_account_id, reviewed_by;
    double amount;
    string status, note, created_at, reviewed_at;
};
```

Four functions:

| Function | SQL |
|----------|-----|
| `create` | `INSERT INTO transactions (from, to, amount, note)` — status defaults to `pending` |
| `findPending` | `SELECT * WHERE status = 'pending' ORDER BY created_at` |
| `findByAccountId` | `SELECT * WHERE from_account_id = ? OR to_account_id = ?` |
| `updateStatus` | `UPDATE SET status, reviewed_by, reviewed_at WHERE id = ?` |

**Why `reviewed_by` is `int` defaulting to `0`:**
SQLite stores it as `NULL` when not reviewed. When converting the row to the struct, an empty string from the query maps to `0`, which signals "not yet reviewed" to callers without needing `std::optional<int>`.

---

## Libraries Used

### `sqlite3` (SQLite 3.45.1 — bundled amalgamation)
**Location:** `lib/sqlite3/sqlite3.c` + `sqlite3.h`

**Why bundled instead of a system install?**
The project needs to build identically on any team member's machine without requiring a separate install step. The SQLite amalgamation is a single `.c` file (~9MB) that compiles directly into the project. No `apt install`, no vcpkg, no CMake `find_package` that might resolve a different version.

**Key SQLite APIs used:**

| API | Where used | Why |
|-----|-----------|-----|
| `sqlite3_open` | `DB::DB()` | Opens/creates the `.db` file |
| `sqlite3_close` | `DB::~DB()` | Closes the connection in the destructor |
| `sqlite3_exec` | `DB::execute(sql)` | Simple one-shot execution for DDL and PRAGMAs |
| `sqlite3_prepare_v2` | `DB::execute(binder)`, `DB::runQuery()` | Prepared statements for all parameterized queries |
| `sqlite3_bind_int/text/double` | Models | Bind values to `?` placeholders safely |
| `sqlite3_step` | `DB::runQuery()` | Iterates rows of a SELECT result |
| `sqlite3_column_name/text` | `DB::runQuery()` | Reads column names and values from each row |
| `sqlite3_finalize` | Both query paths | Releases the prepared statement |
| `sqlite3_last_insert_rowid` | `DB::lastInsertId()` | Gets the id of the last inserted row |
| `sqlite3_errmsg` | All error paths | Readable error message for exceptions |

**Why WAL mode (`PRAGMA journal_mode = WAL`)?**
WAL (Write-Ahead Logging) allows concurrent reads while a write is in progress. For a single-process console app this doesn't matter much, but it also avoids the risk of database corruption if the process is killed mid-write — safer than the default rollback journal.

**Why `PRAGMA foreign_keys = ON`?**
SQLite does not enforce foreign key constraints by default — this must be explicitly enabled per connection. Turning it on ensures that, for example, you cannot insert a transaction referencing a non-existent account.

---

### Standard Library Headers Used

| Header | Why |
|--------|-----|
| `<string>` | All text fields in structs and SQL strings |
| `<vector>` | Return type for list queries and row collections |
| `<map>` | `DB::Row` — maps column names to string values |
| `<optional>` | Return type for single-row lookups (nullopt = not found) |
| `<functional>` | `std::function<void(sqlite3_stmt*)>` — the binder callback type |
| `<stdexcept>` | `std::runtime_error` thrown on all DB/SQL failures |
| `<cstdint>` | `uint64_t` in FNV-1a hash, `int64_t` for last insert id |
| `<cstdio>` | `snprintf` for account number formatting |
| `<iomanip>` | `setw`, `setprecision`, `fixed` for smoke-test output |
| `<iostream>` | `cout`/`cerr` for smoke-test output |

---

## What Dev 2 Needs From Dev 1

Dev 2 (Auth System) can now use:

```cpp
#include "database/db.h"        // DB class
#include "database/schema.h"    // Schema::init(db)
#include "models/user.h"        // User struct + UserModel::
#include "models/account.h"     // Account struct + AccountModel::
#include "auth/password.h"      // Password::hash() / Password::verify()
```

The `Password::hash()` / `Password::verify()` functions are implemented as a minimal FNV-1a stub in `src/auth/password.cpp`. Dev 2 **must not change the algorithm** without re-seeding the admin account, since the admin's stored hash was generated using this algorithm when the database was first created.

---

## Smoke Test Output (verified 2026-05-29)

```
=== Banking Management System — Dev 1 Smoke Test ===

SQLite version : 3.45.1
Database file  : banking.db

--- Users (1) ---
ID  Name                Username        Role
------------------------------------------------
1   Administrator       admin           admin

--- Creating test user 'alice' ---
User created    : Alice Johnson (id=2)
Account created : ACC000002  balance=$0.00

--- Accounts (1) ---
ID    Account No.   Balance
------------------------------
1     ACC000002     $0.00

Dev 1 layer OK — all models operational.
```
