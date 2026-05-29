#pragma once
#include <string>

// Minimal implementation provided by Dev 1 to support schema seeding.
// Dev 2 owns this module and may enhance the algorithm — but MUST keep
// hash() and verify() consistent with each other.
namespace Password {
    std::string hash  (const std::string& plain);
    bool        verify(const std::string& plain, const std::string& stored_hash);
}
