# System Architecture

## Overview

Console-based Banking Management System written in C++17 with SQLite as the
data store. No web server, no networking — pure terminal UI with `std::cin /
std::cout`.

---

## User Roles

| Role  | Can Do |
|-------|--------|
| User  | Register, Login, Send money, View balance, View transaction history |
| Admin | Login, View all pending transactions, Approve / Reject transactions, View all users |

---

## Data Flow

```
┌────────────────────────────────────────────────────────┐
│                        main.cpp                        │
│  Shows: 1) Login  2) Register  3) Exit                 │
│  After login → routes to UserMenu or AdminMenu         │
└────────────────────┬───────────────────────────────────┘
                     │
          ┌──────────┴──────────┐
          ▼                     ▼
   ┌─────────────┐       ┌─────────────┐
   │  UserMenu   │       │  AdminMenu  │
   │  (Dev 3)    │       │  (Dev 4)    │
   └──────┬──────┘       └──────┬──────┘
          │                     │
     ┌────┴────┐           ┌────┴────┐
     │Transfer │           │Approval │
     │  View   │           │UserMgmt │
     └────┬────┘           └────┬────┘
          │                     │
          └──────────┬──────────┘
                     ▼
          ┌──────────────────────┐
          │   Models (Dev 1)     │
          │  User / Account /    │
          │  Transaction         │
          └──────────┬───────────┘
                     ▼
          ┌──────────────────────┐
          │  DB + Schema (Dev 1) │
          │  sqlite3 wrapper     │
          └──────────────────────┘
```

---

## Transaction Lifecycle

```
User sends money
      │
      ▼
Transaction created → status: PENDING
      │
      ▼
Admin logs in → sees all PENDING transactions
      │
      ├── Approves → sender balance decreases
      │              receiver balance increases
      │              status: APPROVED
      │
      └── Rejects → no balance change
                    status: REJECTED
```

---

## Module Boundaries

```
auth/       ← only module allowed to create sessions
models/     ← only layer allowed to touch sqlite3 directly
user/       ← reads Session, reads/writes via models
admin/      ← reads Session, reads/writes via models
main.cpp    ← only file that includes auth/ and both menus
```

No module should include sqlite3.h directly except `database/db.h`.

---

## Console UI Flow

```
=== Banking Management System ===
1. Login
2. Register
3. Exit
> _

--- After login as USER ---
=== User Menu ===
1. View Balance & History
2. Send Money
3. Logout
> _

--- After login as ADMIN ---
=== Admin Menu ===
1. View Pending Transactions
2. Approve / Reject Transaction
3. View All Users
4. Logout
> _
```
