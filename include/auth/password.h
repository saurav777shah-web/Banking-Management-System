// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : include/auth/password.h
// ABOUT : Password hashing and verification. No plain-text passwords should
//         ever be stored. Keep the implementation in password.cpp only.
//
// MUST PROVIDE:
//   namespace Password
//     - std::string hash(const std::string& plain)
//         → returns a hex digest of the password
//     - bool verify(const std::string& plain, const std::string& stored_hash)
//         → returns true if plain hashes to stored_hash
//
// NOTE  : Use a simple but consistent algorithm (e.g. a multi-round SHA-256
//         or XOR-salt approach). The exact algorithm doesn't matter as long as
//         hash() and verify() are consistent with each other.
//
// DEPENDENCIES: none (pure utility)
// =============================================================================
#pragma once
