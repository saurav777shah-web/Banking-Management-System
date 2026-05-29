# Diagrams

All diagrams use [Mermaid](https://mermaid.js.org/) and render automatically on GitHub.

## Table of Contents
1. [Entity Relationship Diagram (ERD)](#1-entity-relationship-diagram-erd)
2. [C++ Data Structs & Model Methods](#2-c-data-structs--model-methods)
3. [Module Dependency Graph](#3-module-dependency-graph)
4. [Layer & Dev Ownership Map](#4-layer--dev-ownership-map)
5. [Sequence — Register](#5-sequence--register)
6. [Sequence — Login & Route](#6-sequence--login--route)
7. [Sequence — Send Money](#7-sequence--send-money)
8. [Sequence — Approve Transaction](#8-sequence--approve-transaction)
9. [Sequence — Reject Transaction](#9-sequence--reject-transaction)
10. [Transaction State Machine](#10-transaction-state-machine)

---

## 1. Entity Relationship Diagram (ERD)

```mermaid
erDiagram
    users {
        int     id            PK
        text    name
        text    username      UK
        text    password_hash
        text    role
        text    created_at
    }

    accounts {
        int     id            PK
        int     user_id       FK
        text    account_number UK
        real    balance
        text    created_at
    }

    transactions {
        int     id              PK
        int     from_account_id FK
        int     to_account_id   FK
        real    amount
        text    status
        text    note
        text    created_at
        int     reviewed_by     FK
        text    reviewed_at
    }

    users    ||--o|   accounts     : "owns (1 user = 0..1 account)"
    users    ||--o{   transactions : "reviews (admin only)"
    accounts ||--o{   transactions : "sends from"
    accounts ||--o{   transactions : "receives into"
```

### Constraint Reference

| Table | Column | Constraint | Effect |
|-------|--------|-----------|--------|
| users | username | UNIQUE | No duplicate logins |
| users | role | CHECK IN ('user','admin') | Only valid roles |
| accounts | balance | CHECK >= 0 | Balance never goes negative at DB level |
| accounts | account_number | UNIQUE | No duplicate account numbers |
| transactions | amount | CHECK > 0 | Zero / negative transfers blocked |
| transactions | status | CHECK IN ('pending','approved','rejected') | Only valid states |
| All FK cols | REFERENCES + FK pragma ON | Orphan rows impossible |

---

## 2. C++ Data Structs & Model Methods

```mermaid
classDiagram
    direction TB

    class User {
        +int id
        +string name
        +string username
        +string password_hash
        +string role
        +string created_at
    }

    class UserModel {
        <<namespace>>
        +findById(db, id) optional~User~
        +findByUsername(db, username) optional~User~
        +create(db, name, username, hash, role) User
        +findAll(db) vector~User~
    }

    class Account {
        +int id
        +int user_id
        +string account_number
        +double balance
        +string created_at
    }

    class AccountModel {
        <<namespace>>
        +create(db, user_id) Account
        +findByUserId(db, user_id) optional~Account~
        +findByAccountNumber(db, acc_no) optional~Account~
        +updateBalance(db, account_id, new_balance) bool
        +findAll(db) vector~Account~
    }

    class Transaction {
        +int id
        +int from_account_id
        +int to_account_id
        +double amount
        +string status
        +string note
        +string created_at
        +int reviewed_by
        +string reviewed_at
    }

    class TransactionModel {
        <<namespace>>
        +create(db, from_id, to_id, amount, note) Transaction
        +findPending(db) vector~Transaction~
        +findByAccountId(db, account_id) vector~Transaction~
        +updateStatus(db, txn_id, status, reviewed_by) bool
    }

    class SessionData {
        +int user_id
        +string username
        +string role
        +int account_id
    }

    class Session {
        <<namespace>>
        +start(data) void
        +end() void
        +isLoggedIn() bool
        +current() SessionData
    }

    class DB {
        +execute(sql) void
        +execute(sql, binder) void
        +query(sql) vector~Row~
        +query(sql, binder) vector~Row~
        +lastInsertId() int64
        +handle() sqlite3ptr
    }

    UserModel    ..>  User        : returns
    AccountModel ..>  Account     : returns
    TransactionModel ..> Transaction : returns
    Session      ..>  SessionData : stores / returns

    UserModel        -->  DB : uses
    AccountModel     -->  DB : uses
    TransactionModel -->  DB : uses

    User        "1" --> "0..1" Account     : owns
    Account     "1" --> "0..*" Transaction : from
    Account     "1" --> "0..*" Transaction : to
    User        "1" --> "0..*" Transaction : reviews
```

---

## 3. Module Dependency Graph

```mermaid
flowchart TD
    classDef presentation fill:#3B82F6,color:#fff,stroke:#1D4ED8
    classDef auth         fill:#F59E0B,color:#fff,stroke:#B45309
    classDef model        fill:#10B981,color:#fff,stroke:#065F46
    classDef db           fill:#8B5CF6,color:#fff,stroke:#5B21B6
    classDef entry        fill:#EF4444,color:#fff,stroke:#991B1B

    MAIN([main.cpp]):::entry

    subgraph P["Presentation Layer"]
        UM[UserMenu]:::presentation
        TR[Transfer]:::presentation
        UA[UserAccount]:::presentation
        AM[AdminMenu]:::presentation
        AP[Approval]:::presentation
        MG[UserMgmt]:::presentation
    end

    subgraph A["Auth Layer"]
        AUTH[Auth]:::auth
        SESS[Session]:::auth
        PASS[Password]:::auth
    end

    subgraph M["Model Layer"]
        UMOD[UserModel]:::model
        AMOD[AccountModel]:::model
        TMOD[TransactionModel]:::model
    end

    subgraph D["Database Layer"]
        DB[DB class]:::db
        SCH[Schema]:::db
        SQ[(sqlite3\nbundled)]:::db
    end

    MAIN --> UM & AM & AUTH & DB & SCH

    UM  --> TR & UA & SESS
    AM  --> AP & MG & SESS

    TR  --> AMOD & TMOD & SESS
    UA  --> AMOD & TMOD & SESS
    AP  --> AMOD & TMOD & SESS & DB
    MG  --> UMOD & AMOD

    AUTH --> UMOD & AMOD & PASS & SESS & DB
    SCH  --> DB & PASS

    UMOD --> DB
    AMOD --> DB
    TMOD --> DB
    DB   --> SQ
```

---

## 4. Layer & Dev Ownership Map

```mermaid
flowchart TB
    classDef dev1  fill:#6366F1,color:#fff,stroke:#4338CA
    classDef dev2  fill:#EC4899,color:#fff,stroke:#BE185D
    classDef dev3  fill:#14B8A6,color:#fff,stroke:#0F766E
    classDef dev4  fill:#F97316,color:#fff,stroke:#C2410C
    classDef sqlite fill:#64748B,color:#fff,stroke:#334155

    subgraph L4["⬛ Layer 4 — Presentation"]
        direction LR
        subgraph D2M["Dev 2"]
            MAIN[main.cpp]:::dev2
        end
        subgraph D3M["Dev 3"]
            UM[UserMenu]:::dev3
            TR[Transfer]:::dev3
            UA[UserAccount]:::dev3
        end
        subgraph D4M["Dev 4"]
            AM[AdminMenu]:::dev4
            AP[Approval]:::dev4
            MG[UserMgmt]:::dev4
        end
    end

    subgraph L3["⬛ Layer 3 — Auth / Session  (Dev 2)"]
        direction LR
        AUTH[Auth]:::dev2
        SESS[Session]:::dev2
        PASS[Password]:::dev2
    end

    subgraph L2["⬛ Layer 2 — Models  (Dev 1)"]
        direction LR
        UMOD[UserModel]:::dev1
        AMOD[AccountModel]:::dev1
        TMOD[TransactionModel]:::dev1
    end

    subgraph L1["⬛ Layer 1 — Database  (Dev 1)"]
        direction LR
        DB[DB class]:::dev1
        SCH[Schema]:::dev1
        SQ[(sqlite3)]:::sqlite
    end

    L4 --> L3 --> L2 --> L1
```

---

## 5. Sequence — Register

```mermaid
sequenceDiagram
    actor       U  as User
    participant M  as main.cpp
    participant A  as Auth
    participant UM as UserModel
    participant AM as AccountModel
    participant P  as Password
    participant DB as DB / SQLite

    U  ->>  M  : name, username, password
    M  ->>  A  : registerUser(db, name, username, password)
    A  ->>  UM : findByUsername(db, username)
    UM ->>  DB : SELECT * FROM users WHERE username=?
    DB -->> UM : (empty)
    UM -->> A  : nullopt — username is free

    A  ->>  P  : hash(password)
    P  -->> A  : "a3f2b1c9..." (hex hash)

    A  ->>  UM : create(db, name, username, hash, "user")
    UM ->>  DB : INSERT INTO users ...
    DB -->> UM : id = 2
    UM -->> A  : User { id=2 }

    A  ->>  AM : create(db, user_id=2)
    AM ->>  DB : INSERT INTO accounts (user_id=2, acc_no="ACC000002")
    DB -->> AM : id = 1
    AM -->> A  : Account { number="ACC000002" }

    A  -->> M  : true
    M  -->> U  : "Registration successful! Account: ACC000002"
```

---

## 6. Sequence — Login & Route

```mermaid
sequenceDiagram
    actor       U  as User
    participant M  as main.cpp
    participant A  as Auth
    participant UM as UserModel
    participant AM as AccountModel
    participant P  as Password
    participant S  as Session
    participant Mn as UserMenu or AdminMenu

    U  ->>  M  : username, password
    M  ->>  A  : loginUser(db, username, password)
    A  ->>  UM : findByUsername(db, username)
    UM -->> A  : User { id, role, password_hash, ... }

    A  ->>  P  : verify(password, stored_hash)
    P  -->> A  : true

    alt role == "user"
        A  ->>  AM : findByUserId(db, user.id)
        AM -->> A  : Account { id=1 }
        A  ->>  S  : start({ user_id, username, "user", account_id=1 })
    else role == "admin"
        A  ->>  S  : start({ user_id, username, "admin", account_id=0 })
    end

    A  -->> M  : true
    M  ->>  S  : current().role

    alt "user"
        M  ->>  Mn : UserMenu::run(db)
    else "admin"
        M  ->>  Mn : AdminMenu::run(db)
    end

    Mn -->> M  : (user logged out — loop back)
```

---

## 7. Sequence — Send Money

```mermaid
sequenceDiagram
    actor       U  as User
    participant UM as UserMenu
    participant T  as Transfer
    participant S  as Session
    participant AM as AccountModel
    participant TM as TransactionModel
    participant DB as DB / SQLite

    U  ->>  UM : "2. Send Money"
    UM ->>  T  : sendMoney(db)

    T  ->>  S  : current()
    S  -->> T  : { user_id=2, account_id=1 }

    T  ->>  AM : findByUserId(db, user_id=2)
    AM -->> T  : Account { id=1, balance=1250.00 }

    T  -->> U  : "Enter recipient account number:"
    U  ->>  T  : "ACC000005"
    T  ->>  AM : findByAccountNumber(db, "ACC000005")
    AM -->> T  : Account { id=5, balance=200.00 }

    T  -->> U  : "Enter amount:"
    U  ->>  T  : 500

    Note over T : Validate: recipient exists ✓<br/>not self ✓<br/>amount > 0 ✓<br/>balance 1250 >= 500 ✓

    T  -->> U  : "Enter note (optional):"
    U  ->>  T  : "rent"

    T  ->>  TM : create(db, from_id=1, to_id=5, amount=500, note="rent")
    TM ->>  DB : INSERT INTO transactions ... status='pending'
    DB -->> TM : id = 3
    TM -->> T  : Transaction { id=3, status="pending" }

    T  -->> U  : "Transaction #3 is pending admin approval."
    Note over DB : No balance change yet.<br/>Balances only move on admin approval.
```

---

## 8. Sequence — Approve Transaction

```mermaid
sequenceDiagram
    actor       Ad as Admin
    participant AM as AdminMenu
    participant AP as Approval
    participant S  as Session
    participant TM as TransactionModel
    participant Ac as AccountModel
    participant DB as DB / SQLite

    Ad ->>  AM : "2. Approve / Reject"
    AM ->>  AP : reviewTransaction(db)

    AP ->>  TM : findPending(db)
    TM ->>  DB : SELECT * FROM transactions WHERE status='pending'
    DB -->> TM : [ Transaction #3 ... ]
    TM -->> AP : list of pending transactions
    AP -->> Ad : (shows pending list)

    Ad ->>  AP : Transaction ID = 3

    AP ->>  Ac : findByAccountNumber(db, "ACC000002")
    Ac -->> AP : Sender { id=1, balance=1250.00 }
    AP ->>  Ac : findByAccountNumber(db, "ACC000005")
    Ac -->> AP : Receiver { id=5, balance=200.00 }

    AP -->> Ad : shows transaction details
    Ad ->>  AP : "A" (Approve)

    Note over AP : Re-check: sender.balance 1250 >= amount 500 ✓

    AP ->>  DB : BEGIN
    AP ->>  Ac : updateBalance(db, sender_id=1, 1250-500=750)
    Ac ->>  DB : UPDATE accounts SET balance=750 WHERE id=1
    AP ->>  Ac : updateBalance(db, receiver_id=5, 200+500=700)
    Ac ->>  DB : UPDATE accounts SET balance=700 WHERE id=5
    AP ->>  S  : current().user_id
    S  -->> AP : admin_id = 1
    AP ->>  TM : updateStatus(db, txn_id=3, "approved", reviewed_by=1)
    TM ->>  DB : UPDATE transactions SET status='approved', reviewed_by=1, reviewed_at=now() WHERE id=3
    AP ->>  DB : COMMIT

    DB -->> AP : OK
    AP -->> Ad : "Transaction #3 approved. $500.00 transferred."
```

---

## 9. Sequence — Reject Transaction

```mermaid
sequenceDiagram
    actor       Ad as Admin
    participant AP as Approval
    participant S  as Session
    participant TM as TransactionModel
    participant DB as DB / SQLite

    Ad ->>  AP : Transaction ID = 3, action = "R" (Reject)

    AP ->>  S  : current().user_id
    S  -->> AP : admin_id = 1

    AP ->>  TM : updateStatus(db, txn_id=3, "rejected", reviewed_by=1)
    TM ->>  DB : UPDATE transactions SET status='rejected',<br/>reviewed_by=1, reviewed_at=now() WHERE id=3
    DB -->> TM : OK
    TM -->> AP : true

    AP -->> Ad : "Transaction #3 rejected."
    Note over DB : No balance change.<br/>reviewed_by and reviewed_at are recorded<br/>for the audit trail.
```

---

## 10. Transaction State Machine

```mermaid
stateDiagram-v2
    direction LR

    [*] --> PENDING : Transfer::sendMoney()\ncreates transaction row

    PENDING --> APPROVED : Admin approves\n──────────────\nsender.balance  -= amount\nreceiver.balance += amount\nreviewed_by = admin_id\nreviewed_at = now()

    PENDING --> REJECTED : Admin rejects\n──────────────\nno balance change\nreviewed_by = admin_id\nreviewed_at = now()

    APPROVED --> [*] : terminal state
    REJECTED --> [*] : terminal state

    note right of PENDING
        Balance check happens TWICE:
        1. At sendMoney (sender has enough?)
        2. At approve (re-checked in case
           another txn drained the balance)
    end note
```
