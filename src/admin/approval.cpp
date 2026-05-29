// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : src/admin/approval.cpp
// ABOUT : Implement Approval::listPending() and Approval::reviewTransaction().
//
// TASKS:
//   [ ] listPending(db)
//         → fetch all pending transactions (TransactionModel::findPending)
//         → if none: print "No pending transactions."
//         → else print table:
//             ID | Date                | From Acc   | To Acc     | Amount  | Note
//             ---|---------------------|------------|------------|---------|-----
//             3  | 2024-05-01 10:23:00 | ACC000001  | ACC000002  | $500.00 | rent
//
//   [ ] reviewTransaction(db)
//         → call listPending(db) first so admin sees what's available
//         → prompt "Enter Transaction ID to review (0 to cancel): "
//         → fetch the specific transaction, verify it is still "pending"
//         → show transaction details
//         → prompt "Action: (A)pprove / (R)eject / (C)ancel: "
//         → on Approve:
//             - check sender still has sufficient balance
//             - AccountModel::updateBalance(sender, sender.balance - amount)
//             - AccountModel::updateBalance(receiver, receiver.balance + amount)
//             - TransactionModel::updateStatus(id, "approved", admin_id)
//             - print "Transaction #X approved."
//         → on Reject:
//             - TransactionModel::updateStatus(id, "rejected", admin_id)
//             - print "Transaction #X rejected."
//
// IMPORTANT: Approve must update BOTH balances AND status atomically.
//            Use db.execute("BEGIN") / db.execute("COMMIT") to wrap them.
// =============================================================================
#include "admin/approval.h"
