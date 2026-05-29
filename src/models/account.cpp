// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : src/models/account.cpp
// ABOUT : Implement all AccountModel functions declared in include/models/account.h.
//
// TASKS:
//   [ ] create(db, user_id)
//         → generate account number: "ACC" + zero-padded 6-digit id, e.g. "ACC000001"
//         → INSERT INTO accounts (user_id, account_number, balance) ...
//         → return the newly created Account
//   [ ] findByUserId(db, user_id)
//         → SELECT * FROM accounts WHERE user_id = ?
//         → return std::optional<Account>
//   [ ] findByAccountNumber(db, account_number)
//         → SELECT * FROM accounts WHERE account_number = ?
//         → return std::optional<Account>
//   [ ] updateBalance(db, account_id, new_balance)
//         → UPDATE accounts SET balance = ? WHERE id = ?
//         → return true on success
//   [ ] findAll(db)
//         → SELECT * FROM accounts ORDER BY id
//         → return std::vector<Account>
// =============================================================================
#include "models/account.h"
