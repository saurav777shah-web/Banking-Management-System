// Minimal implementation by Dev 1 — Dev 2 may enhance but must keep
// hash() and verify() consistent with each other.
#include "auth/password.h"
#include <cstdint>
#include <cstdio>

namespace Password {

std::string hash(const std::string& plain) {
    // FNV-1a 64-bit with a fixed salt prefix
    const std::string salted = "BMS$$" + plain + "$$2024";
    uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : salted) {
        h ^= static_cast<uint64_t>(c);
        h *= 1099511628211ULL;
    }
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llx",
                  static_cast<unsigned long long>(h));
    return std::string(buf);
}

bool verify(const std::string& plain, const std::string& stored_hash) {
    return hash(plain) == stored_hash;
}

} // namespace Password
