// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : src/models/transaction.cpp
// ABOUT : Implement all TransactionModel functions declared in
//         include/models/transaction.h.
//
// TASKS:
//   [ ] create(db, from_id, to_id, amount, note)
//         → INSERT INTO transactions (from_account_id, to_account_id, amount, note)
//         → status defaults to "pending" (DB default)
//         → return the newly created Transaction
//   [ ] findPending(db)
//         → SELECT * FROM transactions WHERE status = 'pending' ORDER BY created_at
//         → return std::vector<Transaction>
//   [ ] findByAccountId(db, account_id)
//         → SELECT * FROM transactions
//             WHERE from_account_id = ? OR to_account_id = ?
//             ORDER BY created_at DESC
//         → return std::vector<Transaction>
//   [ ] updateStatus(db, txn_id, status, reviewed_by)
//         → UPDATE transactions
//             SET status = ?, reviewed_by = ?, reviewed_at = datetime('now')
//             WHERE id = ?
//         → return true on success
// =============================================================================
#include "models/transaction.h"
