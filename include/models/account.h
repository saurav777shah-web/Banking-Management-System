#pragma once
#include <string>
#include <optional>
#include <vector>
#include "database/db.h"

struct Account {
    int         id             = 0;
    int         user_id        = 0;
    std::string account_number;
    double      balance        = 0.0;
    std::string created_at;
};

namespace AccountModel {
    Account                create              (DB& db, int user_id);
    std::optional<Account> findByUserId        (DB& db, int user_id);
    std::optional<Account> findByAccountNumber (DB& db, const std::string& account_number);
    bool                   updateBalance       (DB& db, int account_id, double new_balance);
    std::vector<Account>   findAll             (DB& db);
}
