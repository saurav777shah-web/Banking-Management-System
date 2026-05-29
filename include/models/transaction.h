#pragma once
#include <string>
#include <vector>
#include "database/db.h"

struct Transaction {
    int         id              = 0;
    int         from_account_id = 0;
    int         to_account_id   = 0;
    double      amount          = 0.0;
    std::string status;          // "pending" | "approved" | "rejected"
    std::string note;
    std::string created_at;
    int         reviewed_by     = 0;  // admin user id, 0 if not yet reviewed
    std::string reviewed_at;
};

namespace TransactionModel {
    Transaction              create          (DB& db, int from_account_id,
                                                      int to_account_id,
                                                      double amount,
                                                      const std::string& note = "");
    std::vector<Transaction> findPending     (DB& db);
    std::vector<Transaction> findByAccountId (DB& db, int account_id);
    bool                     updateStatus    (DB& db, int txn_id,
                                                      const std::string& status,
                                                      int reviewed_by);
}
