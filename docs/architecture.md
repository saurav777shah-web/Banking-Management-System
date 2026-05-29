# System Architecture

## Table of Contents
1. [Project Overview](#1-project-overview)
2. [Tech Stack](#2-tech-stack)
3. [User Roles & Permissions](#3-user-roles--permissions)
4. [Layered Architecture](#4-layered-architecture)
5. [Module Breakdown](#5-module-breakdown)
6. [Include Dependency Graph](#6-include-dependency-graph)
7. [Data Flow — Every Operation](#7-data-flow--every-operation)
8. [Transaction Lifecycle](#8-transaction-lifecycle)
9. [Session & State Model](#9-session--state-model)
10. [Database Overview](#10-database-overview)
11. [Error Handling Strategy](#11-error-handling-strategy)
12. [Console UI — All Screens](#12-console-ui--all-screens)
13. [Module Boundary Rules](#13-module-boundary-rules)
14. [Key Design Decisions](#14-key-design-decisions)

---

## 1. Project Overview

A **console-based** Banking Management System built in C++17.
No web server. No GUI. No networking. Pure terminal I/O via `std::cin` / `std::cout`.

Data is persisted in a single SQLite file (`banking.db`) created automatically
on first run in the working directory.

**Core behaviour in one paragraph:**
A new user registers through the terminal, which creates a user record and a
bank account with zero balance. They can then log in and submit money transfers
to other accounts. Every transfer starts as `PENDING` — no money moves until
an admin reviews and approves it. On approval, both account balances are updated
atomically inside a database transaction. On rejection, nothing changes. The
admin can also view all users and their balances at any time.

---

## 2. Tech Stack

| Component | Choice | Why |
|-----------|--------|-----|
| Language | C++17 | `std::optional`, structured bindings, inline variables |
| Database | SQLite 3.45.1 | Zero-install, single file, full SQL, battle-tested |
| SQLite delivery | Bundled amalgamation (`lib/sqlite3/sqlite3.c`) | No system install needed — same version on every machine |
| Build system | CMake 3.16+ | Cross-platform, generates MinGW Makefiles on Windows |
| Compiler | GCC 15 (MinGW) | Already on PATH, C++17 support |
| Standard I/O | `std::cin` / `std::cout` | No curses or Windows console API needed |

**What was deliberately not used:**
- No ORM — raw SQL keeps the logic visible and the build simple
- No Boost — standard library covers everything needed
- No threading — single-threaded, no concurrency concerns
- No network layer — all operations are local

---

## 3. User Roles & Permissions

### Regular User
Created via the Register screen. Gets one bank account automatically.

| Action | Allowed | Notes |
|--------|---------|-------|
| Register | ✅ | Creates user row + account row |
| Login | ✅ | Starts session with role = `"user"` |
| View own balance | ✅ | Reads `accounts` table |
| View own transaction history | ✅ | Reads `transactions` table filtered by account |
| Send money | ✅ | Creates a `PENDING` transaction row |
| Approve/reject transactions | ❌ | Admin only |
| View all users | ❌ | Admin only |

### Admin
Seeded automatically on first run. Has no bank account — exists only to manage
the system.

| Action | Allowed | Notes |
|--------|---------|-------|
| Register | ❌ | Admin accounts are seeded, not self-registered |
| Login | ✅ | Starts session with role = `"admin"` |
| View pending transactions | ✅ | Reads all `PENDING` rows in `transactions` |
| Approve a transaction | ✅ | Updates both balances + sets status `APPROVED` |
| Reject a transaction | ✅ | Sets status `REJECTED`, no balance change |
| View all users + accounts | ✅ | Reads `users` JOIN `accounts` |
| Send money | ❌ | Admin has no account |

**Default admin credentials** (seeded on first run):
```
username : admin
password : admin123
```

---

## 4. Layered Architecture

The system is split into four strict layers. A layer may only talk to the layer
directly below it — never skip a layer or talk sideways.

```
┌─────────────────────────────────────────────────────────────────┐
│                      PRESENTATION LAYER                         │
│                                                                 │
│   main.cpp          UserMenu          AdminMenu                 │
│   (login loop)      (Dev 3)           (Dev 4)                   │
│                     Transfer          Approval                  │
│                     UserAccount       UserMgmt                  │
│                                                                 │
│   What it does: reads user input, prints output, calls          │
│   business logic. No SQL here. No direct DB access.             │
└───────────────────────────────┬─────────────────────────────────┘
                                │ calls
┌───────────────────────────────▼─────────────────────────────────┐
│                      AUTH / SESSION LAYER                       │
│                                                                 │
│   Auth::registerUser()     Auth::loginUser()                    │
│   Session::start()         Session::current()                   │
│   Session::end()           Password::hash/verify()              │
│                                                                 │
│   What it does: validates credentials, manages who is logged    │
│   in, hashes passwords. Calls the model layer.                  │
└───────────────────────────────┬─────────────────────────────────┘
                                │ calls
┌───────────────────────────────▼─────────────────────────────────┐
│                        MODEL LAYER                              │
│                                                                 │
│   UserModel::       AccountModel::      TransactionModel::      │
│   findById          create              create                  │
│   findByUsername    findByUserId        findPending             │
│   create            findByAccountNumber findByAccountId         │
│   findAll           updateBalance       updateStatus            │
│                     findAll                                     │
│                                                                 │
│   What it does: translates between C++ structs and SQL rows.    │
│   All queries are parameterised. Returns std::optional / vec.   │
└───────────────────────────────┬─────────────────────────────────┘
                                │ calls
┌───────────────────────────────▼─────────────────────────────────┐
│                      DATABASE LAYER                             │
│                                                                 │
│   DB::execute(sql)              DB::query(sql)                  │
│   DB::execute(sql, binder)      DB::query(sql, binder)          │
│   DB::lastInsertId()            Schema::init(db)                │
│                                                                 │
│   What it does: wraps raw sqlite3 API. Every other layer        │
│   uses this — no other file ever calls sqlite3_* directly.      │
│                                                                 │
│                    [ sqlite3.c — bundled ]                      │
│                    [ banking.db — on disk ]                     │
└─────────────────────────────────────────────────────────────────┘
```

---

## 5. Module Breakdown

### `database/` — DB Wrapper (Dev 1)
**Entry point for the entire codebase.** Everything starts with `DB db("banking.db")`.

- `DB::DB(path)` — calls `sqlite3_open`, enables WAL mode and foreign keys
- `DB::execute(sql)` — uses `sqlite3_exec` for simple DDL / no-param DML
- `DB::execute(sql, binder)` — uses prepared statement + lambda to bind params
- `DB::query(sql)` / `DB::query(sql, binder)` — returns `vector<map<string,string>>`
- `Schema::init(db)` — creates 3 tables (`IF NOT EXISTS`) and seeds admin

> No other file should include `<sqlite3.h>` — only `db.cpp` and `schema.cpp`.

---

### `models/` — Structs + CRUD (Dev 1)
Three plain structs that mirror the database tables exactly, plus namespaced
functions to read and write them.

**Why plain structs instead of classes?**
Models are pure data — no behaviour, no invariants to protect. A struct with
a namespace of free functions (`UserModel::findById`) is simpler to read and
test than a class with member functions.

**Why `std::optional<T>` for single-row lookups?**
A lookup can legitimately return nothing (user not found, account doesn't exist).
Returning `std::optional` forces the caller to handle the empty case explicitly
at compile time, rather than checking for a null pointer at runtime.

```
UserModel::    →  users table       →  User struct
AccountModel:: →  accounts table    →  Account struct
TransactionModel:: → transactions table → Transaction struct
```

---

### `auth/` — Authentication + Session (Dev 2)
Two separate concerns bundled in one module:

**Authentication (`auth.cpp`)** — knows about the database. Calls models.
- `registerUser` — hashes password, creates User row, creates Account row
- `loginUser` — verifies password, starts session

**Session (`session.cpp`)** — knows nothing about the database. Pure memory.
- A single static `SessionData` variable holds the logged-in user's info
- Any module can call `Session::current()` to get it without passing it around
- Lives for the duration of one login → logout cycle

**Password (`password.cpp`)** — knows nothing about database or session. Pure function.
- `hash(plain)` → hex string
- `verify(plain, stored)` → bool

---

### `user/` — User Features (Dev 3)
Three tightly related components, all gated behind a valid session with `role == "user"`.

- `UserMenu` — the menu loop. Reads input, calls Transfer or UserAccount.
- `Transfer` — the send-money flow. Validates input, creates PENDING transaction.
- `UserAccount` — shows balance and full transaction history.

> Transfer must never modify balances. It only creates a transaction row.

---

### `admin/` — Admin Panel (Dev 4)
Three components gated behind a valid session with `role == "admin"`.

- `AdminMenu` — the menu loop. Reads input, calls Approval or UserMgmt.
- `Approval` — shows pending transactions, handles approve/reject with atomicity.
- `UserMgmt` — shows all users and their account details.

> Approval::reviewTransaction wraps balance updates in `BEGIN/COMMIT` — the
> most critical correctness requirement in the project.

---

## 6. Include Dependency Graph

Who includes whom. Arrows point from includer to included.

```
main.cpp
  ├── database/db.h
  ├── database/schema.h
  ├── auth/auth.h
  ├── auth/session.h
  ├── user/user_menu.h
  └── admin/admin_menu.h

auth/auth.h
  ├── database/db.h
  ├── models/user.h
  ├── models/account.h
  ├── auth/password.h
  └── auth/session.h

user/user_menu.h        user/transfer.h         user/user_account.h
  ├── database/db.h       ├── database/db.h        ├── database/db.h
  ├── auth/session.h      ├── models/account.h     ├── models/account.h
  ├── user/transfer.h     ├── models/transaction.h └── models/transaction.h
  └── user/user_account.h └── auth/session.h

admin/admin_menu.h      admin/approval.h         admin/user_mgmt.h
  ├── database/db.h       ├── database/db.h        ├── database/db.h
  ├── auth/session.h      ├── models/account.h     ├── models/user.h
  ├── admin/approval.h    ├── models/transaction.h └── models/account.h
  └── admin/user_mgmt.h   └── auth/session.h

models/user.h           models/account.h         models/transaction.h
  └── database/db.h       └── database/db.h         └── database/db.h
```

**Rule:** `database/db.h` is at the bottom — nothing it includes should ever
reach back up the chain. Circular includes will cause compile errors.

---

## 7. Data Flow — Every Operation

### Register
```
main.cpp
  │  prompts: name, username, password
  ▼
Auth::registerUser(db, name, username, password)
  │  checks: UserModel::findByUsername → must be nullopt (username free)
  │  hashes: Password::hash(password)
  │  writes: UserModel::create(db, name, username, hash, "user")
  │  writes: AccountModel::create(db, user.id)     → generates ACC000XXX
  ▼
main.cpp prints "Registration successful. Account: ACC000XXX"
```

### Login
```
main.cpp
  │  prompts: username, password
  ▼
Auth::loginUser(db, username, password)
  │  reads:   UserModel::findByUsername(db, username) → must exist
  │  checks:  Password::verify(password, user.password_hash) → must be true
  │  reads:   AccountModel::findByUserId(db, user.id)  [skipped for admin]
  │  writes:  Session::start({ user_id, username, role, account_id })
  ▼
main.cpp checks Session::current().role
  ├── "user"  → UserMenu::run(db)
  └── "admin" → AdminMenu::run(db)
```

### View Balance & History
```
UserAccount::showBalanceAndHistory(db)
  │  reads: Session::current() → gets account_id
  │  reads: AccountModel::findByUserId(db, user_id)   → current balance
  │  reads: TransactionModel::findByAccountId(db, account_id)
  ▼
Prints balance + table of all transactions (sent + received + pending)
```

### Send Money
```
Transfer::sendMoney(db)
  │  reads:   Session::current() → sender's account_id
  │  reads:   AccountModel::findByUserId(db, user_id) → sender account
  │  prompts: recipient account number
  │  reads:   AccountModel::findByAccountNumber(db, input) → recipient
  │  validates: recipient exists, not self, amount > 0, sender.balance >= amount
  │  prompts: amount, optional note
  │  writes:  TransactionModel::create(db, from_id, to_id, amount, note)
  │                                     └── status defaults to "pending"
  ▼
Prints "Transaction #N is pending admin approval."
  NOTE: No balance change happens here.
```

### Approve Transaction
```
Approval::reviewTransaction(db)
  │  reads:    TransactionModel::findPending(db) → shows list
  │  prompts:  transaction ID
  │  reads:    finds the transaction in pending list
  │  reads:    AccountModel::findByAccountNumber → sender + receiver
  │  validates: sender.balance >= amount (re-checked at approve time)
  │
  │  db.execute("BEGIN;")
  │  writes: AccountModel::updateBalance(db, sender.id,   sender.balance - amount)
  │  writes: AccountModel::updateBalance(db, receiver.id, receiver.balance + amount)
  │  writes: TransactionModel::updateStatus(db, txn_id, "approved", admin_id)
  │  db.execute("COMMIT;")
  ▼
Prints "Transaction #N approved. $X.XX transferred."
```

### Reject Transaction
```
Approval::reviewTransaction(db)
  │  (same steps as approve up to the action prompt)
  │
  │  writes: TransactionModel::updateStatus(db, txn_id, "rejected", admin_id)
  ▼
Prints "Transaction #N rejected."
  NOTE: No balance change. reviewed_by and reviewed_at are recorded.
```

### Logout
```
UserMenu::run  or  AdminMenu::run
  │  user selects Logout
  ▼
Session::end()   ← clears the static session variable
  ▼
menu function returns to main.cpp
  ▼
main.cpp loops back to the login screen
```

---

## 8. Transaction Lifecycle

Every money transfer goes through exactly three possible states:

```
                      ┌─────────────┐
  Transfer::          │             │
  sendMoney()  ──────▶│   PENDING   │
                      │             │
                      └──────┬──────┘
                             │
                    Admin reviews it
                             │
              ┌──────────────┴──────────────┐
              │                             │
              ▼                             ▼
     ┌─────────────────┐          ┌──────────────────┐
     │    APPROVED      │          │    REJECTED       │
     │                 │          │                  │
     │ sender.balance  │          │ no balance       │
     │   -= amount     │          │ change           │
     │ receiver.balance│          │                  │
     │   += amount     │          │ reviewed_by and  │
     │ reviewed_by set │          │ reviewed_at set  │
     └─────────────────┘          └──────────────────┘
```

**States are terminal** — once a transaction is `APPROVED` or `REJECTED`,
it can never change again. `updateStatus` should only be called on `PENDING`
transactions. Dev 4 must verify this before writing.

**The balance check happens twice:**
1. At `Transfer::sendMoney` — prevents submitting a transfer the user can't cover.
2. At `Approval::reviewTransaction` — re-checked at approve time because
   another transaction might have been approved between submission and review,
   draining the sender's balance.

---

## 9. Session & State Model

The session is the only piece of **in-memory global state** in the system.
Everything else either comes from the database or is a local variable.

```
┌────────────────────────────────────────────────────────┐
│                  session.cpp (static)                  │
│                                                        │
│   static bool        active = false                    │
│   static SessionData current_session = {}              │
│                                                        │
│   SessionData fields:                                  │
│     int         user_id     ← DB primary key of user  │
│     std::string username    ← for display purposes     │
│     std::string role        ← "user" or "admin"        │
│     int         account_id  ← 0 for admin (no account) │
└────────────────────────────────────────────────────────┘
```

**Lifecycle:**
```
App starts → active = false
Login      → Session::start(data) → active = true
Any module → Session::current()   → returns current_session
Logout     → Session::end()       → active = false
App loops  → back to login screen
```

**Why a global static instead of passing a session parameter everywhere?**
Without a global session, every function signature would need a `const Session&`
parameter — `UserMenu::run(db, session)`, `Transfer::sendMoney(db, session)`,
`Approval::reviewTransaction(db, session)`, etc. This creates noise and makes
refactoring harder. A single global read-only session is a standard pattern
for console applications with one active user at a time.

---

## 10. Database Overview

Three tables. See [database-schema.md](database-schema.md) for the full SQL.

```
users
  ├── id (PK)
  ├── name, username (UNIQUE), password_hash
  ├── role: "user" | "admin"
  └── created_at

accounts
  ├── id (PK)
  ├── user_id → users.id  (FK, one account per user)
  ├── account_number (UNIQUE, e.g. "ACC000002")
  ├── balance (REAL, CHECK >= 0)
  └── created_at

transactions
  ├── id (PK)
  ├── from_account_id → accounts.id  (FK)
  ├── to_account_id   → accounts.id  (FK)
  ├── amount (REAL, CHECK > 0)
  ├── status: "pending" | "approved" | "rejected"
  ├── note (optional)
  ├── created_at
  ├── reviewed_by → users.id  (FK, nullable — null until reviewed)
  └── reviewed_at (nullable — null until reviewed)
```

**Foreign keys are enforced.** `PRAGMA foreign_keys = ON` is set in `DB::DB()`.
You cannot insert a transaction referencing an account that doesn't exist.

**WAL mode is on.** `PRAGMA journal_mode = WAL` is set in `DB::DB()`. This
protects against database corruption if the process is killed mid-write.

---

## 11. Error Handling Strategy

```
sqlite3 API returns error codes
        │
        ▼
DB class catches them → throws std::runtime_error("message")
        │
        ▼
Model functions let exceptions propagate up
        │
        ▼
Auth / menu functions may catch specific errors and show a friendly message
        │
        ▼
main.cpp wraps everything in try { } catch (std::exception& e) { }
  → prints "Fatal: <message>" and exits cleanly
```

**Two categories of errors:**

| Category | Examples | How to handle |
|----------|---------|---------------|
| Fatal / unexpected | DB file can't open, SQL syntax error, disk full | Let it propagate to `main.cpp` catch block — print and exit |
| Expected / user-caused | Username taken, account not found, insufficient funds | Catch locally, print friendly message, re-prompt — never crash |

**Never `exit()` or `abort()` from inside a menu function.** Always return to
the caller and let `main.cpp` control the exit path.

---

## 12. Console UI — All Screens

### Main Screen (everyone sees this first)
```
================================================
     Banking Management System
================================================
  1. Login
  2. Register
  3. Exit
> _
```

### Register Screen
```
--- Register ---
Full name    : _
Username     : _
Password     : _

Registration successful! Your account number is: ACC000003
```

### Login Screen
```
--- Login ---
Username : _
Password : _

[Invalid username or password.]   ← shown on failure, re-prompt
```

### User Menu (after user login)
```
================================================
  Welcome, Alice Johnson
  Account : ACC000002   |   Balance : $1,250.00
================================================
  1. View Balance & Transaction History
  2. Send Money
  3. Logout
> _
```

### View Balance & History Screen
```
╔══════════════════════════════════════╗
  Account   : ACC000002
  Balance   : $1,250.00
╚══════════════════════════════════════╝

Transaction History:
ID   Date                  From          To            Amount     Status
---- --------------------- ------------- ------------- ---------- ----------
3    2024-05-01 10:23:00   ACC000002(you) ACC000005    $500.00    pending
1    2024-04-15 09:00:00   ACC000001     ACC000002(you) $2,000.00 approved
```

### Send Money Screen
```
--- Send Money ---
Recipient account number : ACC000005
Amount ($)               : 500
Note (optional)          : rent

Transfer submitted.
Transaction #3 is pending admin approval.
```

### Admin Menu (after admin login)
```
================================================
  Admin Panel — Welcome, Administrator
================================================
  1. View Pending Transactions
  2. Approve / Reject a Transaction
  3. View All Users & Accounts
  4. Logout
> _
```

### View Pending Transactions Screen
```
--- Pending Transactions ---
ID   Date                  From          To            Amount     Note
---- --------------------- ------------- ------------- ---------- ----------
3    2024-05-01 10:23:00   ACC000002     ACC000005     $500.00    rent
5    2024-05-02 14:10:00   ACC000003     ACC000001     $200.00

[2 pending transactions]
```

### Approve / Reject Screen
```
--- Review Transaction ---
[pending list shown above]

Enter Transaction ID to review (0 to cancel): 3

Transaction #3
  From   : ACC000002 (Alice Johnson)
  To     : ACC000005 (Bob Smith)
  Amount : $500.00
  Note   : rent
  Date   : 2024-05-01 10:23:00

Action: (A)pprove  (R)eject  (C)ancel : A

Transaction #3 approved. $500.00 transferred.
```

### View All Users Screen
```
--- All Users & Accounts ---
ID   Name               Username     Account No.    Balance       Role    Joined
---- ------------------ ------------ -------------- ------------- ------- --------------------
1    Administrator      admin        —              —             admin   2024-01-01 00:00:00
2    Alice Johnson      alice        ACC000002      $750.00       user    2024-05-01 08:00:00
3    Bob Smith          bob          ACC000003      $1,200.00     user    2024-05-01 08:05:00
```

---

## 13. Module Boundary Rules

These rules exist to keep the codebase understandable and prevent spaghetti dependencies.

| Rule | Reason |
|------|--------|
| Only `db.cpp` and `schema.cpp` call `sqlite3_*` directly | One place to fix all DB bugs |
| Only `auth/session.cpp` holds session state | One place to debug login/logout issues |
| Only `auth/auth.cpp` calls `Session::start()` | Login logic is never scattered |
| Every menu calls `Session::end()` on logout | Session is always cleaned up |
| `main.cpp` is the only file that includes both `user_menu.h` and `admin_menu.h` | Routing logic lives in one place |
| `Transfer::sendMoney` never modifies balances | Balance changes only happen on admin approval |
| Models never include auth headers | Model layer has zero knowledge of who is logged in |
| `user/` modules never include `admin/` headers and vice versa | Feature modules are fully isolated |

---

## 14. Key Design Decisions

### Why SQLite instead of a custom file format?
Writing binary files or CSV means reimplementing filtering, sorting, and
concurrent-safe writes from scratch. SQLite provides all of that, runs
without a server, needs no installation, and is a single `.c` file that
compiles in under 10 seconds.

### Why the binder-lambda pattern in DB::execute/query?
```cpp
db.execute(
    "INSERT INTO users (name, username) VALUES (?, ?);",
    [&](sqlite3_stmt* s) {
        sqlite3_bind_text(s, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(s, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    }
);
```
String interpolation (`"INSERT ... VALUES ('" + name + "')"`) is vulnerable
to SQL injection and breaks on names with apostrophes. Prepared statements
with `?` placeholders are the correct approach. The lambda pattern keeps the
binding code right next to the query string that it belongs to.

### Why one account per user, not multiple?
Multiple accounts per user adds a selection step everywhere — "which of your
accounts do you want to send from?" For a first version, one account per user
keeps every flow simple. The schema supports multiple (no UNIQUE on user_id
beyond the first) — it can be extended later.

### Why `BEGIN/COMMIT` only on approve, not on every operation?
SQLite auto-wraps every single statement in an implicit transaction. For single
statements (insert a user, update a balance) this is safe. The approve flow
is the only case where **two separate updates must succeed or fail together**
— deducting from sender and crediting receiver. That's the only place an
explicit transaction is needed.

### Why FNV-1a for password hashing instead of bcrypt/Argon2?
This is a student/learning project with no real-world users. FNV-1a has no
external dependencies, compiles in under 1 second, and is consistent across
runs. A production system would use bcrypt or Argon2id — the `password.cpp`
module is intentionally isolated so swapping the algorithm is a one-file change.
