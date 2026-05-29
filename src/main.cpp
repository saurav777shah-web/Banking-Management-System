// TODO (Dev 2): Replace this smoke-test with the full login/register loop.
#include <iostream>
#include <iomanip>
#include "database/db.h"
#include "database/schema.h"
#include "models/user.h"
#include "models/account.h"
#include "models/transaction.h"

// Dev 2 will add these once auth is implemented:
// #include "auth/auth.h"
// #include "auth/session.h"
// #include "user/user_menu.h"
// #include "admin/admin_menu.h"

int main() {
    try {
        DB db("banking.db");
        Schema::init(db);

        std::cout << "=== Banking Management System — Dev 1 Smoke Test ===\n\n";
        std::cout << "SQLite version : " << sqlite3_libversion() << "\n";
        std::cout << "Database file  : banking.db\n\n";

        // Show all users (should include seeded admin)
        auto users = UserModel::findAll(db);
        std::cout << "--- Users (" << users.size() << ") ---\n";
        std::cout << std::left
                  << std::setw(4)  << "ID"
                  << std::setw(20) << "Name"
                  << std::setw(16) << "Username"
                  << std::setw(8)  << "Role"
                  << "\n"
                  << std::string(48, '-') << "\n";
        for (const auto& u : users)
            std::cout << std::setw(4)  << u.id
                      << std::setw(20) << u.name
                      << std::setw(16) << u.username
                      << std::setw(8)  << u.role
                      << "\n";

        // Create a test user + account to verify models work
        std::cout << "\n--- Creating test user 'alice' ---\n";
        auto existing = UserModel::findByUsername(db, "alice");
        if (!existing) {
            auto u = UserModel::create(db, "Alice Johnson", "alice", "hashed_pw", "user");
            auto a = AccountModel::create(db, u.id);
            std::cout << "User created    : " << u.name << " (id=" << u.id << ")\n";
            std::cout << "Account created : " << a.account_number
                      << "  balance=$" << std::fixed << std::setprecision(2) << a.balance << "\n";
        } else {
            std::cout << "User 'alice' already exists (re-run skipped).\n";
        }

        // Show accounts
        auto accounts = AccountModel::findAll(db);
        std::cout << "\n--- Accounts (" << accounts.size() << ") ---\n";
        std::cout << std::setw(6)  << "ID"
                  << std::setw(14) << "Account No."
                  << std::setw(10) << "Balance"
                  << "\n"
                  << std::string(30, '-') << "\n";
        for (const auto& a : accounts)
            std::cout << std::setw(6)  << a.id
                      << std::setw(14) << a.account_number
                      << "$" << std::fixed << std::setprecision(2) << a.balance
                      << "\n";

        std::cout << "\nDev 1 layer OK — all models operational.\n";

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
