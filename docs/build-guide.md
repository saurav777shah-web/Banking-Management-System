# Build Guide

## Requirements

| Tool | Version | Where |
|------|---------|-------|
| g++ (MinGW) | 15.2+ | Already on PATH |
| cmake | 4.1+ | Already on PATH |
| mingw32-make | any | Already on PATH |

SQLite 3.45.1 is bundled in `lib/sqlite3/` — no download needed.

---

## Quick Build (Windows)

```bat
build.bat
```

The script creates `build/`, runs CMake with MinGW Makefiles, and compiles.
Output binary: `build/banking_system.exe`

---

## Manual Build

```bat
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
mingw32-make -j4
```

---

## Run

```bat
build\banking_system.exe
```

The database file `banking.db` is created in whichever directory you run the
exe from.

---

## Clean Build

```bat
rmdir /s /q build
build.bat
```

---

## Default Credentials (seeded on first run)

| Username | Password | Role  |
|----------|----------|-------|
| admin    | admin123 | Admin |

Register new accounts through the app to create user accounts.
