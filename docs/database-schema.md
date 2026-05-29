# Database Schema

All tables live in a single file: `banking.db` (created at runtime in the
working directory). Dev 1 owns the creation SQL in `src/database/schema.cpp`.

> **Visual ERD with cardinality and constraint table** → [diagrams.md § 1](diagrams.md#1-entity-relationship-diagram-erd)

---

## Table: users

Stores both regular users and admins. Role distinguishes them.

```sql
CREATE TABLE IF NOT EXISTS users (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    name          TEXT    NOT NULL,
    username      TEXT    NOT NULL UNIQUE,
    password_hash TEXT    NOT NULL,
    role          TEXT    NOT NULL CHECK(role IN ('user', 'admin')),
    created_at    TEXT    NOT NULL DEFAULT (datetime('now'))
);
```

---

## Table: accounts

One account per user. Account number is a unique string (e.g. "ACC000001").

```sql
CREATE TABLE IF NOT EXISTS accounts (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id        INTEGER NOT NULL REFERENCES users(id),
    account_number TEXT    NOT NULL UNIQUE,
    balance        REAL    NOT NULL DEFAULT 0.0 CHECK(balance >= 0),
    created_at     TEXT    NOT NULL DEFAULT (datetime('now'))
);
```

---

## Table: transactions

Records every money transfer. Status is managed by the admin.

```sql
CREATE TABLE IF NOT EXISTS transactions (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    from_account_id INTEGER NOT NULL REFERENCES accounts(id),
    to_account_id   INTEGER NOT NULL REFERENCES accounts(id),
    amount          REAL    NOT NULL CHECK(amount > 0),
    status          TEXT    NOT NULL DEFAULT 'pending'
                            CHECK(status IN ('pending', 'approved', 'rejected')),
    note            TEXT,
    created_at      TEXT    NOT NULL DEFAULT (datetime('now')),
    reviewed_by     INTEGER REFERENCES users(id),
    reviewed_at     TEXT
);
```

---

## Seed Data (on first run)

Dev 1 inserts a default admin in `schema.cpp` if no admin exists:

| Field    | Value       |
|----------|-------------|
| name     | Administrator |
| username | admin       |
| password | admin123    |
| role     | admin       |

---

## Relationships

```
users ──< accounts ──< transactions (from_account_id)
                   ──< transactions (to_account_id)
users ──< transactions (reviewed_by)
```

For the full annotated ERD with cardinality labels and a constraint table
see [diagrams.md § 1](diagrams.md#1-entity-relationship-diagram-erd).
