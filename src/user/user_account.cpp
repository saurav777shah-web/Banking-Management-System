// =============================================================================
// OWNER : Dev 3 — User Features
// FILE  : src/user/user_account.cpp
// ABOUT : Implement UserAccount::showBalanceAndHistory() declared in
//         include/user/user_account.h.
//
// TASKS:
//   [ ] Fetch account via Session::current().account_id
//   [ ] Print:
//         Account Number : ACC000XXX
//         Current Balance: $1,234.56
//   [ ] Fetch all transactions for this account (TransactionModel::findByAccountId)
//   [ ] Print transaction table — if no transactions: print "No transactions yet."
//         Format:
//           ID  | Date                | From       | To         | Amount  | Status
//           ----|---------------------|------------|------------|---------|----------
//           1   | 2024-05-01 10:23:00 | ACC000001  | ACC000002  | $500.00 | pending
//   [ ] For each row, mark "(you)" next to the user's own account number
// =============================================================================
#include "user/user_account.h"
