// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : src/database/schema.cpp
// ABOUT : Implement Schema::init() declared in include/database/schema.h.
//
// TASKS:
//   [ ] Create table `users`        (see docs/database-schema.md)
//   [ ] Create table `accounts`     (see docs/database-schema.md)
//   [ ] Create table `transactions` (see docs/database-schema.md)
//   [ ] Seed default admin if no admin exists:
//         name="Administrator", username="admin", password="admin123", role="admin"
//         Call Password::hash() for the password — do NOT store plain text
//
// NOTE: All CREATE TABLE statements use "IF NOT EXISTS" so this is safe
//       to call every time the app starts.
// =============================================================================
#include "database/schema.h"
