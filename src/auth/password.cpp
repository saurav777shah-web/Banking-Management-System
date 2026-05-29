// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : src/auth/password.cpp
// ABOUT : Implement Password::hash() and Password::verify().
//
// TASKS:
//   [ ] hash(plain)
//         → take a plain-text string, return a deterministic hex string
//         → you may implement a simple custom hash or use any standard approach
//         → the algorithm must be consistent — same input always gives same output
//   [ ] verify(plain, stored_hash)
//         → return hash(plain) == stored_hash
//
// SUGGESTED approach (no external libs needed):
//   Use a FNV-1a or djb2 hash seeded with a fixed salt, then hex-encode.
//   Or implement a minimal SHA-256 manually.
//   Consistency matters more than cryptographic strength for this project.
// =============================================================================
#include "auth/password.h"
