# Diagrams

## Table of Contents
1. [Entity Relationship Diagram (ERD)](#1-entity-relationship-diagram-erd)
2. [C++ Data Struct Diagram](#2-c-data-struct-diagram)
3. [Module Dependency Diagram](#3-module-dependency-diagram)
4. [Layer & Ownership Map](#4-layer--ownership-map)
5. [Call Chain — Register](#5-call-chain--register)
6. [Call Chain — Login & Route](#6-call-chain--login--route)
7. [Call Chain — Send Money](#7-call-chain--send-money)
8. [Call Chain — Approve Transaction](#8-call-chain--approve-transaction)

---

## 1. Entity Relationship Diagram (ERD)

Three tables. `users` is the root — everything else hangs off it.

```
┌─────────────────────────────────────────────┐
│                    users                    │
├─────────────────────────────────────────────┤
│  PK  id             INTEGER AUTOINCREMENT   │
│      name           TEXT    NOT NULL        │
│      username       TEXT    NOT NULL UNIQUE │
│      password_hash  TEXT    NOT NULL        │
│      role           TEXT    user | admin    │
│      created_at     TEXT    datetime        │
└──────────────────┬──────────────────────────┘
                   │                │
         1         │                │ 1
         ┌─────────┘                └──────────────────────┐
         │ owns                               reviews (0..*) │
         │ 0..1                                              │
┌────────▼─────────────────────────────┐                    │
│               accounts               │                    │
├──────────────────────────────────────┤                    │
│  PK  id              INTEGER         │                    │
│  FK  user_id    ──▶  users.id        │                    │
│      account_number  TEXT UNIQUE     │                    │
│      balance         REAL  >= 0      │                    │
│      created_at      TEXT            │                    │
└──────────┬───────────────────────────┘                    │
           │                   │                            │
           │ 1 (sends)         │ 1 (receives)               │
           │                   │                            │
           │ 0..*              │ 0..*                       │
┌──────────▼───────────────────▼────────────────────────────▼──┐
│                         transactions                          │
├───────────────────────────────────────────────────────────────┤
│  PK  id               INTEGER                                 │
│  FK  from_account_id ──▶ accounts.id   (sender)               │
│  FK  to_account_id   ──▶ accounts.id   (receiver)             │
│      amount           REAL  > 0                               │
│      status           TEXT  pending | approved | rejected     │
│      note             TEXT  nullable                          │
│      created_at       TEXT  datetime                          │
│  FK  reviewed_by  ──▶ users.id         nullable until reviewed│
│      reviewed_at      TEXT  nullable until reviewed           │
└───────────────────────────────────────────────────────────────┘
```

### Cardinality Summary

| Relationship | Type | Meaning |
|-------------|------|---------|
| users → accounts | 1 : 0..1 | One user owns zero or one account (admins have none) |
| accounts → transactions (from) | 1 : 0..* | One account can send many transactions |
| accounts → transactions (to) | 1 : 0..* | One account can receive many transactions |
| users → transactions (reviewed_by) | 1 : 0..* | One admin can review many transactions |

### Constraints Enforced by the DB

| Column | Constraint | What it prevents |
|--------|-----------|-----------------|
| `users.username` | UNIQUE | Two users with the same login name |
| `users.role` | CHECK IN ('user','admin') | Any role other than the two valid ones |
| `accounts.balance` | CHECK >= 0 | Balance going negative at the DB level |
| `accounts.account_number` | UNIQUE | Two accounts with the same number |
| `transactions.amount` | CHECK > 0 | Zero or negative transfers |
| `transactions.status` | CHECK IN (...) | Any status outside the three valid states |
| All FK columns | REFERENCES + FK pragma ON | Orphan rows — e.g. a transaction with no account |

---

## 2. C++ Data Struct Diagram

The three structs that travel between every layer of the app.
All fields are public value types — no methods, no private state.

```
┌──────────────────────────────────────┐
│          struct User                 │
├──────────────────────────────────────┤
│  id            : int                 │
│  name          : std::string         │
│  username      : std::string         │
│  password_hash : std::string         │
│  role          : std::string         │  ← "user" | "admin"
│  created_at    : std::string         │
└────────────────────────┬─────────────┘
                         │ UserModel::create() returns User
                         │ UserModel::findById() returns optional<User>
                         │ UserModel::findAll() returns vector<User>


┌──────────────────────────────────────┐
│          struct Account              │
├──────────────────────────────────────┤
│  id             : int                │
│  user_id        : int                │  ← FK → User.id
│  account_number : std::string        │  ← "ACC000001"
│  balance        : double             │
│  created_at     : std::string        │
└────────────────────────┬─────────────┘
                         │ AccountModel::create() returns Account
                         │ AccountModel::findByUserId() returns optional<Account>
                         │ AccountModel::findAll() returns vector<Account>


┌──────────────────────────────────────┐
│        struct Transaction            │
├──────────────────────────────────────┤
│  id              : int               │
│  from_account_id : int               │  ← FK → Account.id
│  to_account_id   : int               │  ← FK → Account.id
│  amount          : double            │
│  status          : std::string       │  ← "pending"|"approved"|"rejected"
│  note            : std::string       │
│  created_at      : std::string       │
│  reviewed_by     : int               │  ← FK → User.id  (0 = not reviewed)
│  reviewed_at     : std::string       │
└────────────────────────┬─────────────┘
                         │ TransactionModel::create() returns Transaction
                         │ TransactionModel::findPending() returns vector<Transaction>
                         │ TransactionModel::findByAccountId() returns vector<Transaction>


┌──────────────────────────────────────┐
│        struct SessionData            │
│        (not persisted — memory only) │
├──────────────────────────────────────┤
│  user_id    : int                    │  ← mirrors User.id
│  username   : std::string            │
│  role       : std::string            │  ← "user" | "admin"
│  account_id : int                    │  ← 0 for admin (no account)
└──────────────────────────────────────┘
                         │ Session::start(data) stores this
                         │ Session::current() returns this
```

### Struct Relationships

```
        User  ────────────────────  Account
          1                            0..1
          │ user_id FK                 │ from_account_id FK
          │                            │ to_account_id FK
          └────────────────────  Transaction
          1  reviewed_by FK          0..*
```

---

## 3. Module Dependency Diagram

Arrows mean "this module calls / includes the other".
Modules on the left depend on modules they point to.

```
                    ┌────────────────────────────────────────────┐
                    │                 main.cpp                   │
                    └───┬────────┬───────────┬───────────────────┘
                        │        │           │
               ┌────────▼──┐  ┌──▼────────┐  │
               │ UserMenu  │  │ AdminMenu │  │
               └──┬─────┬──┘  └──┬─────┬──┘  │
                  │     │        │     │      │
           ┌──────▼┐  ┌─▼──────┐ │  ┌──▼──┐  │
           │Trans- │  │UserAcc-│ │  │App- │  │
           │fer    │  │ount    │ │  │roval│  │
           └───┬───┘  └───┬────┘ │  └──┬──┘  │
               │           │     │     │      │
           ┌───▼───────────▼─────▼─────▼──┐   │
           │        UserMgmt              │   │
           └───────────────┬──────────────┘   │
                           │                  │
                    ┌──────▼──────────────────▼──┐
                    │           Auth              │
                    └──────┬──────────────────────┘
                           │
              ┌────────────┼─────────────────┐
              │            │                 │
       ┌──────▼──┐   ┌─────▼────┐    ┌───────▼──────┐
       │ Session │   │ Password │    │  DB class    │
       │(memory) │   │(hash/    │    │  + Schema    │
       │         │   │ verify)  │    │  (sqlite3)   │
       └─────────┘   └──────────┘    └──────┬───────┘
                                            │
                               ┌────────────┼──────────────┐
                               │            │              │
                        ┌──────▼──┐  ┌──────▼──┐  ┌────────▼────┐
                        │UserModel│  │Acct-    │  │Transaction- │
                        │         │  │Model    │  │Model        │
                        └─────────┘  └─────────┘  └─────────────┘
```

### Full Dependency Table

| Module | Directly Depends On |
|--------|-------------------|
| `main.cpp` | DB, Schema, Auth, Session, UserMenu, AdminMenu |
| `UserMenu` | Session, Transfer, UserAccount |
| `AdminMenu` | Session, Approval, UserMgmt |
| `Transfer` | DB, Session, AccountModel, TransactionModel |
| `UserAccount` | Session, AccountModel, TransactionModel |
| `Approval` | DB, Session, AccountModel, TransactionModel |
| `UserMgmt` | UserModel, AccountModel |
| `Auth` | DB, Session, Password, UserModel, AccountModel |
| `Session` | *(none — pure in-memory state)* |
| `Password` | *(none — pure function)* |
| `UserModel` | DB |
| `AccountModel` | DB |
| `TransactionModel` | DB |
| `DB` | sqlite3 (bundled C library) |
| `Schema` | DB, Password |

---

## 4. Layer & Ownership Map

```
╔══════════════════════════════════════════════════════════════════╗
║  LAYER 4 — PRESENTATION                                          ║
║                                                                  ║
║   ┌───────────┐   ┌──────────────────────────────────────────┐  ║
║   │ main.cpp  │   │              Dev 3                        │  ║
║   │           │   │  UserMenu  │  Transfer  │  UserAccount   │  ║
║   │  Dev 2    │   └──────────────────────────────────────────┘  ║
║   └───────────┘                                                  ║
║                   ┌──────────────────────────────────────────┐  ║
║                   │              Dev 4                        │  ║
║                   │  AdminMenu  │  Approval  │  UserMgmt     │  ║
║                   └──────────────────────────────────────────┘  ║
╠══════════════════════════════════════════════════════════════════╣
║  LAYER 3 — AUTH / SESSION                    Dev 2              ║
║                                                                  ║
║       Auth::registerUser / loginUser                             ║
║       Session::start / end / current / isLoggedIn                ║
║       Password::hash / verify                                    ║
╠══════════════════════════════════════════════════════════════════╣
║  LAYER 2 — MODELS                            Dev 1              ║
║                                                                  ║
║       UserModel::        AccountModel::      TransactionModel::  ║
║       findById           create              create              ║
║       findByUsername     findByUserId        findPending         ║
║       create             findByAccNumber     findByAccountId     ║
║       findAll            updateBalance       updateStatus        ║
║                          findAll                                 ║
╠══════════════════════════════════════════════════════════════════╣
║  LAYER 1 — DATABASE                          Dev 1              ║
║                                                                  ║
║       DB::execute / query / lastInsertId / handle                ║
║       Schema::init  →  creates tables  →  seeds admin            ║
║                                                                  ║
║                    [ sqlite3.c — bundled ]                       ║
║                    [ banking.db — on disk ]                       ║
╚══════════════════════════════════════════════════════════════════╝
```

---

## 5. Call Chain — Register

```
User types: name, username, password
         │
         ▼
    main.cpp
         │  Auth::registerUser(db, name, username, password)
         ▼
    auth.cpp
         │  UserModel::findByUsername(db, username)
         ▼
    user.cpp ──▶ db.query("SELECT * FROM users WHERE username=?")
         │
         │  [username already exists?]
         │  YES → return false → main prints "Username taken"
         │
         │  NO  → Password::hash(password)
         ▼
    password.cpp  returns hex string
         │
         │  UserModel::create(db, name, username, hash, "user")
         ▼
    user.cpp ──▶ db.execute("INSERT INTO users ...")
                 db.lastInsertId()  → new user.id = 2
         │
         │  AccountModel::create(db, user.id=2)
         ▼
    account.cpp
         │  generates account_number = "ACC000002"
         │  db.execute("INSERT INTO accounts ...")
         │
         ▼
    auth.cpp returns true
         │
         ▼
    main.cpp prints "Registration successful! Account: ACC000002"
```

---

## 6. Call Chain — Login & Route

```
User types: username, password
         │
         ▼
    main.cpp
         │  Auth::loginUser(db, username, password)
         ▼
    auth.cpp
         │  UserModel::findByUsername(db, username)
         ▼
    user.cpp ──▶ db.query("SELECT * FROM users WHERE username=?")
         │
         │  [user found?]
         │  NO  → return false → main prints "Invalid credentials"
         │
         │  YES → Password::verify(password, user.password_hash)
         ▼
    password.cpp  hashes plain + compares to stored hash
         │
         │  [match?]
         │  NO  → return false → main prints "Invalid credentials"
         │
         │  YES → role == "user"?
         │         YES → AccountModel::findByUserId(db, user.id)
         │         NO  → account_id = 0  (admin has no account)
         │
         │  Session::start({ user_id, username, role, account_id })
         ▼
    session.cpp  stores SessionData in static variable, active=true
         │
         ▼
    auth.cpp returns true
         │
         ▼
    main.cpp checks Session::current().role
         │
         ├── "user"  ──▶  UserMenu::run(db)
         └── "admin" ──▶  AdminMenu::run(db)
```

---

## 7. Call Chain — Send Money

```
User selects: Send Money
         │
         ▼
    user_menu.cpp
         │  Transfer::sendMoney(db)
         ▼
    transfer.cpp
         │  Session::current()  → gets account_id
         │  AccountModel::findByUserId(db, user_id)  → sender account
         │
         │  prompts: "Enter recipient account number:"
         │  AccountModel::findByAccountNumber(db, input)
         │
         │  [recipient exists?]  NO  → "Account not found." → return
         │  [recipient == sender?]   YES → "Cannot send to yourself." → return
         │
         │  prompts: "Enter amount:"
         │  [amount <= 0?]           YES → "Amount must be > 0." → return
         │  [balance < amount?]      YES → "Insufficient funds." → return
         │
         │  prompts: "Enter note (optional):"
         │
         │  TransactionModel::create(db, from_id, to_id, amount, note)
         ▼
    transaction.cpp
         │  db.execute("INSERT INTO transactions ...")
         │            status defaults to 'pending'
         │  db.lastInsertId()  → txn.id = 3
         │
         ▼
    transfer.cpp prints "Transaction #3 is pending admin approval."
         │
         ▼  [NO BALANCE CHANGE — money moves only on admin approval]
    returns to UserMenu loop
```

---

## 8. Call Chain — Approve Transaction

The most critical flow. Two balance updates and a status update
must all succeed or all fail together.

```
Admin selects: Approve / Reject a Transaction
         │
         ▼
    admin_menu.cpp
         │  Approval::reviewTransaction(db)
         ▼
    approval.cpp
         │  Approval::listPending(db)  → prints pending list
         │
         │  prompts: "Enter Transaction ID (0 to cancel):"
         │  input = 3
         │
         │  finds txn #3 in pending list
         │  [found and still pending?]  NO  → "Not found or already reviewed."
         │
         │  displays transaction details
         │  prompts: "(A)pprove  (R)eject  (C)ancel"
         │
         ├─── CANCEL ─────────────────────────────────── return
         │
         ├─── REJECT ──────────────────────────────────────────────────────┐
         │    TransactionModel::updateStatus(db, 3, "rejected", admin_id)  │
         │    db: UPDATE transactions SET status='rejected',               │
         │         reviewed_by=1, reviewed_at=datetime('now') WHERE id=3   │
         │    prints "Transaction #3 rejected."                            │
         │    return ◀───────────────────────────────────────────────────┘
         │
         └─── APPROVE
                  │
                  │  Re-fetch sender account (balance may have changed!)
                  │  AccountModel::findByAccountNumber(db, from_acc_number)
                  │
                  │  [sender.balance >= amount?]
                  │  NO → "Insufficient funds. Cannot approve." → return
                  │
                  │  ┌─────────────────────────────────────────────────┐
                  │  │           ATOMIC DATABASE TRANSACTION           │
                  │  │                                                 │
                  │  │  db.execute("BEGIN;")                           │
                  │  │                                                 │
                  │  │  AccountModel::updateBalance(                   │
                  │  │      db, sender.id,                             │
                  │  │      sender.balance - amount    ← deduct        │
                  │  │  )                                              │
                  │  │  db: UPDATE accounts SET balance=750 WHERE id=2 │
                  │  │                                                 │
                  │  │  AccountModel::updateBalance(                   │
                  │  │      db, receiver.id,                           │
                  │  │      receiver.balance + amount  ← credit        │
                  │  │  )                                              │
                  │  │  db: UPDATE accounts SET balance=1700 WHERE id=5│
                  │  │                                                 │
                  │  │  TransactionModel::updateStatus(                │
                  │  │      db, 3, "approved", admin_id                │
                  │  │  )                                              │
                  │  │  db: UPDATE transactions SET                    │
                  │  │       status='approved', reviewed_by=1,         │
                  │  │       reviewed_at=datetime('now') WHERE id=3    │
                  │  │                                                 │
                  │  │  db.execute("COMMIT;")                          │
                  │  │                                                 │
                  │  │  If ANY step fails → SQLite auto-rolls back     │
                  │  │  all three updates together                     │
                  │  └─────────────────────────────────────────────────┘
                  │
                  ▼
             prints "Transaction #3 approved. $500.00 transferred."
             return to AdminMenu loop
```
