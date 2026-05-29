// =============================================================================
// OWNER : Dev 1 — Database & Models
// FILE  : include/database/schema.h
// ABOUT : Declares schema initialisation. Called once at startup from main.cpp
//         to create all tables and seed the default admin account.
//
// MUST PROVIDE:
//   namespace Schema
//     - void init(DB& db)  → creates tables if not exist, seeds admin if needed
//
// DEPENDENCIES: db.h
// =============================================================================
#pragma once
