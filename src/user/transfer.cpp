// =============================================================================
// OWNER : Dev 3 — User Features
// FILE  : src/user/transfer.cpp
// ABOUT : Implement Transfer::sendMoney() declared in include/user/transfer.h.
//
// TASKS:
//   [ ] Get sender's account via Session::current().account_id
//   [ ] Prompt: "Enter recipient account number: "
//   [ ] Prompt: "Enter amount: "
//   [ ] Prompt: "Enter note (optional, press Enter to skip): "
//   [ ] Validate:
//         - recipient account exists (AccountModel::findByAccountNumber)
//         - recipient != sender
//         - amount > 0
//         - sender balance >= amount  (reject if not enough funds)
//   [ ] Create transaction: TransactionModel::create(db, from_id, to_id, amount, note)
//   [ ] Print: "Transfer submitted. Transaction #X is PENDING admin approval."
//   [ ] On any validation failure: print error and return without creating txn
//
// NOTE: Do NOT modify balances here. Balances change only when admin approves.
// =============================================================================
#include "user/transfer.h"
