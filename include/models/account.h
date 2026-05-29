// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : include/models/account.h
// ABOUT : Account struct and all CRUD helpers for the `accounts` table.
//         Account numbers are generated here (e.g. "ACC000001").
//
// MUST PROVIDE:
//   struct Account
//     - int id
//     - int user_id
//     - std::string account_number
//     - double balance
//     - std::string created_at
//
//   namespace AccountModel
//     - Account                create(DB&, int user_id)
//     - std::optional<Account> findByUserId(DB&, int user_id)
//     - std::optional<Account> findByAccountNumber(DB&, const std::string&)
//     - bool                   updateBalance(DB&, int account_id, double new_balance)
//     - std::vector<Account>   findAll(DB&)
//
// DEPENDENCIES: db.h
// =============================================================================
#pragma once
