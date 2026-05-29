// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : include/models/transaction.h
// ABOUT : Transaction struct and CRUD helpers for the `transactions` table.
//         Status values: "pending", "approved", "rejected"
//
// MUST PROVIDE:
//   struct Transaction
//     - int id
//     - int from_account_id, to_account_id
//     - double amount
//     - std::string status, note, created_at
//     - int reviewed_by  (0 if not yet reviewed)
//     - std::string reviewed_at
//
//   namespace TransactionModel
//     - Transaction              create(DB&, from_id, to_id, amount, note)
//     - std::vector<Transaction> findPending(DB&)
//     - std::vector<Transaction> findByAccountId(DB&, int account_id)
//     - bool                     updateStatus(DB&, int txn_id,
//                                             const std::string& status,
//                                             int reviewed_by)
//
// DEPENDENCIES: db.h
// =============================================================================
#pragma once
