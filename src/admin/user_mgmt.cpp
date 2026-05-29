// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : src/admin/user_mgmt.cpp
// ABOUT : Implement UserMgmt::listAllUsers() declared in
//         include/admin/user_mgmt.h.
//
// TASKS:
//   [ ] Fetch all users (UserModel::findAll)
//   [ ] For each user, fetch their account (AccountModel::findByUserId)
//   [ ] Print table:
//         ID | Name          | Username | Account No. | Balance    | Role  | Joined
//         ---|---------------|----------|-------------|------------|-------|-------------------
//         1  | Administrator | admin    | —           | —          | admin | 2024-01-01 00:00
//         2  | Alice Johnson | alice    | ACC000001   | $1,000.00  | user  | 2024-05-01 10:00
//
//   [ ] Admin accounts have no balance/account — display "—" for those columns
// =============================================================================
#include "admin/user_mgmt.h"
