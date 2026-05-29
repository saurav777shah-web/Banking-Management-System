# Developer Assignments

## Team of 4 — Build Order

Build order matters. Dev 1 must finish first because everyone else depends on
the database and model layer. Dev 2 goes second because Dev 3 and Dev 4 both
need session/auth middleware. Dev 3 and Dev 4 can then work in parallel.

---

## Dev 1 — Database & Models

**Delivers first. Everyone builds on top of this.**

### Owns
| File | Purpose |
|------|---------|
| `include/database/db.h` | DB connection class (open, close, execute, query) |
| `include/database/schema.h` | Table definitions and schema init declaration |
| `include/models/user.h` | User model — struct + CRUD helpers |
| `include/models/account.h` | Account model — struct + CRUD helpers |
| `include/models/transaction.h` | Transaction model — struct + CRUD helpers |
| `src/database/db.cpp` | Implements DB connection wrapper over sqlite3 |
| `src/database/schema.cpp` | Creates all tables on first run, seeds default admin |
| `src/models/user.cpp` | findByUsername, findById, create, list all |
| `src/models/account.cpp` | findByUserId, findByAccountNumber, create, updateBalance |
| `src/models/transaction.cpp` | create, findPending, findByAccount, updateStatus |

### Delivers
- `banking.db` is created and all tables exist on startup
- Model structs usable by all other devs (`User`, `Account`, `Transaction`)
- All CRUD helpers return `std::optional<T>` or `std::vector<T>`
- A default admin account seeded: username `admin`, password `admin123`

---

## Dev 2 — Auth System

**Delivers second. Dev 3 and Dev 4 wait for this.**

### Owns
| File | Purpose |
|------|---------|
| `include/auth/password.h` | Hash and verify passwords |
| `include/auth/session.h` | Holds who is currently logged in (role, user id) |
| `include/auth/auth.h` | Register and login entry-points |
| `src/auth/password.cpp` | Simple SHA-256 or custom hash implementation |
| `src/auth/session.cpp` | Singleton session state, login/logout helpers |
| `src/auth/auth.cpp` | registerUser(), loginUser() — validates input, calls models |
| `src/main.cpp` | Top-level loop: show login/register, route to user or admin menu |

### Delivers
- `Auth::registerUser(username, password, name)` → creates user + account
- `Auth::loginUser(username, password)` → returns `Session` with role + id
- `Session::current()` → gives any module the logged-in user without passing it around
- `main.cpp` main loop wired up: login → branch to user menu or admin menu

### Depends on
- Dev 1: `User` model, `Account` model, `DB` class

---

## Dev 3 — User Features

**Can start after Dev 2 finishes. Works in parallel with Dev 4.**

### Owns
| File | Purpose |
|------|---------|
| `include/user/user_menu.h` | Top-level user console menu |
| `include/user/transfer.h` | Send money to another account |
| `include/user/user_account.h` | View balance and transaction history |
| `src/user/user_menu.cpp` | Prints menu, reads choice, dispatches to Transfer / Account |
| `src/user/transfer.cpp` | Prompts recipient account number + amount, creates PENDING transaction |
| `src/user/user_account.cpp` | Fetches and displays own balance and all past transactions |

### Delivers
- `UserMenu::run()` — called from `main.cpp` when a regular user logs in
- Send money creates a `Transaction` with status `PENDING` (admin must approve)
- User sees their own balance and full history including pending amounts

### Depends on
- Dev 1: `Account`, `Transaction` models
- Dev 2: `Session::current()` to get logged-in user

---

## Dev 4 — Admin Panel

**Can start after Dev 2 finishes. Works in parallel with Dev 3.**

### Owns
| File | Purpose |
|------|---------|
| `include/admin/admin_menu.h` | Top-level admin console menu |
| `include/admin/approval.h` | List, approve, and reject pending transactions |
| `include/admin/user_mgmt.h` | View all users and their account balances |
| `src/admin/admin_menu.cpp` | Prints menu, reads choice, dispatches to Approval / UserMgmt |
| `src/admin/approval.cpp` | Lists all PENDING transactions, admin picks one to approve or reject |
| `src/admin/user_mgmt.cpp` | Fetches and displays all users with account numbers and balances |

### Delivers
- `AdminMenu::run()` — called from `main.cpp` when an admin logs in
- Approving a transaction: deducts from sender, credits receiver, marks APPROVED
- Rejecting a transaction: marks REJECTED, no balance change
- Admin can view every user's account and balance at any time

### Depends on
- Dev 1: `User`, `Account`, `Transaction` models
- Dev 2: `Session::current()` to confirm admin role
