# Dev 2 — Update Log
Role:   Authentication (register, login, password hashing) and session tracking
Date:   02/06/2026
Status: Complete

## Summary
Implemented the complete authentication and session management system for the Banking Management System.

## Completed Tasks

### 1. Reviewed password.cpp
- Reviewed existing FNV-1a hash implementation by Dev 1
- Implementation is consistent and working
- Kept as-is to maintain admin seed hash compatibility

### 2. Implemented Session Management (session.h + session.cpp)
- Created `SessionData` struct with fields:
  - `int user_id` - logged-in user's database id
  - `std::string username` - user's username
  - `std::string role` - "user" or "admin"
  - `int account_id` - user's account id (0 for admins)
- Implemented Session namespace functions:
  - `void start(const SessionData& data)` - starts session
  - `void end()` - ends session
  - `bool isLoggedIn()` - checks if session is active
  - `SessionData current()` - returns current session (throws if not logged in)

### 3. Implemented Authentication (auth.h + auth.cpp)
- Implemented `Auth::registerUser(db, name, username, password)`:
  - Validates empty username/password
  - Checks username availability
  - Hashes password using Password::hash()
  - Creates user with "user" role
  - Creates bank account for new user
  - Prints confirmation with account number
  
- Implemented `Auth::loginUser(db, username, password)`:
  - Looks up user by username
  - Verifies password using Password::verify()
  - Creates SessionData with proper role handling
  - For admins: account_id = 0
  - For users: fetches and stores account_id
  - Calls Session::start() on successful login

### 4. Implemented Main Entry Point (main.cpp)
- Replaced Dev 1's smoke test with full authentication loop
- Features:
  - Database initialization with Schema::init()
  - Main menu loop showing:
    1. Login
    2. Register
    3. Exit
  - Input trimming for better UX
  - Login flow: prompt username/password → validate → route to appropriate menu
  - Register flow: collect name/username/password → create user + account
  - Routes authenticated users to:
    - AdminMenu::run(db) for admins
    - UserMenu::run(db) for regular users
  - Error handling with try-catch for exceptions

### 5. Added Menu Stubs
- Added function declarations to `user/user_menu.h` and `admin/admin_menu.h`
- Provided stub implementations in `user_menu.cpp` and `admin_menu.cpp`
- Allows code to compile and links properly to main.cpp

## Files Modified
- `include/auth/session.h` - SessionData struct + Session namespace
- `src/auth/session.cpp` - Session implementation
- `include/auth/auth.h` - Auth namespace with register/login functions
- `src/auth/auth.cpp` - Auth implementation
- `src/main.cpp` - Full authentication loop
- `include/user/user_menu.h` - Added function declaration
- `src/user/user_menu.cpp` - Stub implementation
- `include/admin/admin_menu.h` - Added function declaration
- `src/admin/admin_menu.cpp` - Stub implementation

## Contract Delivered to Dev 3 & Dev 4
```cpp
#include "auth/session.h"
SessionData s = Session::current();
s.user_id      // int
s.username     // std::string
s.role         // "user" or "admin"
s.account_id   // int (0 for admin)
```

Session is ready for Dev 3 and Dev 4 to use!

## Key Implementation Notes
- Password hashing uses existing FNV-1a algorithm (no database re-init needed)
- Session is in-memory singleton (one user logged in at a time)
- Input validation includes empty string and space trimming
- Main loop handles invalid input gracefully
- Proper error handling with try-catch for database exceptions
- Authentication flow is compatible with DevAdmin login flow described in assignment