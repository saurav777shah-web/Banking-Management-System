#include "models/transaction.h"

static Transaction rowToTransaction(const DB::Row& r) {
    Transaction t;
    t.id              = std::stoi(r.at("id"));
    t.from_account_id = std::stoi(r.at("from_account_id"));
    t.to_account_id   = std::stoi(r.at("to_account_id"));
    t.amount          = std::stod(r.at("amount"));
    t.status          = r.at("status");
    t.note            = r.at("note");
    t.created_at      = r.at("created_at");
    t.reviewed_by     = r.at("reviewed_by").empty() ? 0 : std::stoi(r.at("reviewed_by"));
    t.reviewed_at     = r.at("reviewed_at");
    return t;
}

namespace TransactionModel {

Transaction create(DB& db, int from_account_id, int to_account_id,
                   double amount, const std::string& note) {
    db.execute(
        "INSERT INTO transactions (from_account_id, to_account_id, amount, note)"
        " VALUES (?, ?, ?, ?);",
        [&](sqlite3_stmt* s) {
            sqlite3_bind_int   (s, 1, from_account_id);
            sqlite3_bind_int   (s, 2, to_account_id);
            sqlite3_bind_double(s, 3, amount);
            sqlite3_bind_text  (s, 4, note.c_str(), -1, SQLITE_TRANSIENT);
        }
    );
    int new_id = static_cast<int>(db.lastInsertId());
    auto rows = db.query(
        "SELECT * FROM transactions WHERE id = ? LIMIT 1;",
        [new_id](sqlite3_stmt* s) { sqlite3_bind_int(s, 1, new_id); }
    );
    return rowToTransaction(rows[0]);
}

std::vector<Transaction> findPending(DB& db) {
    auto rows = db.query(
        "SELECT * FROM transactions WHERE status = 'pending' ORDER BY created_at;"
    );
    std::vector<Transaction> txns;
    txns.reserve(rows.size());
    for (const auto& r : rows) txns.push_back(rowToTransaction(r));
    return txns;
}

std::vector<Transaction> findByAccountId(DB& db, int account_id) {
    auto rows = db.query(
        "SELECT * FROM transactions"
        " WHERE from_account_id = ? OR to_account_id = ?"
        " ORDER BY created_at DESC;",
        [account_id](sqlite3_stmt* s) {
            sqlite3_bind_int(s, 1, account_id);
            sqlite3_bind_int(s, 2, account_id);
        }
    );
    std::vector<Transaction> txns;
    txns.reserve(rows.size());
    for (const auto& r : rows) txns.push_back(rowToTransaction(r));
    return txns;
}

bool updateStatus(DB& db, int txn_id, const std::string& status, int reviewed_by) {
    db.execute(
        "UPDATE transactions"
        " SET status = ?, reviewed_by = ?, reviewed_at = datetime('now')"
        " WHERE id = ?;",
        [&](sqlite3_stmt* s) {
            sqlite3_bind_text(s, 1, status.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int (s, 2, reviewed_by);
            sqlite3_bind_int (s, 3, txn_id);
        }
    );
    return true;
}

} // namespace TransactionModel
