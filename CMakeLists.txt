cmake_minimum_required(VERSION 3.16)
project(BankingManagementSystem VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SQLite3 (bundled amalgamation in lib/sqlite3/) ─────────────────────────
add_library(sqlite3 STATIC lib/sqlite3/sqlite3.c)
target_include_directories(sqlite3 PUBLIC lib/sqlite3)
set_target_properties(sqlite3 PROPERTIES C_STANDARD 99)

# ── Collect all source files ───────────────────────────────────────────────
file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/database/*.cpp
    src/models/*.cpp
    src/auth/*.cpp
    src/user/*.cpp
    src/admin/*.cpp
)

# ── Main executable ────────────────────────────────────────────────────────
add_executable(banking_system ${SOURCES})

target_include_directories(banking_system PRIVATE include)
target_link_libraries(banking_system PRIVATE sqlite3)

if(WIN32)
    target_link_libraries(banking_system PRIVATE ws2_32)
endif()

if(MINGW)
    target_link_options(banking_system PRIVATE -static-libgcc -static-libstdc++)
endif()
