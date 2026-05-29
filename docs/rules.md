# Contribution Rules

Every developer must document their work before pushing. Use `updatelog/dev1.md`
as the reference format. Below are the required sections.

---

## Required Sections in Your `updatelog/devN.md`

### 1. Header
```
# Dev N — Update Log
Role:   <your role name>
Date:   <date completed>
Status: Complete / In Progress
```

### 2. What Was Implemented
For each file you wrote, explain:
- **What** the file does (one sentence)
- **Why** you made the key design decisions (not obvious from reading the code)
- A table of the public functions / methods and what SQL or logic they run

Do not just list file names. Explain the reasoning. Someone who joins the team
after you should be able to read your update log and understand *why* the code
looks the way it does — not just *what* it does.

### 3. Libraries Used
For every external library or non-trivial standard-library header:
- **Name and version**
- **Why this library** was chosen over alternatives
- **Which specific APIs** you used and why

### 4. What the Next Dev Needs From You
List the exact `#include` paths and function names that the next dev in the
build order depends on. If you made a contract decision (e.g. a specific hash
algorithm, a specific return type) that downstream code must honour, say so
explicitly here.

### 5. Verified Output
Paste the actual terminal output from a successful build and run.
If your module has no runnable output yet, paste the compiler output showing
zero errors and zero warnings.

---

## General Rules

**One file per dev, named `devN.md`.**
`updatelog/dev1.md`, `updatelog/dev2.md`, etc. Update your own file only.

**Commit message must match your update log.**
The first line of your commit message should summarise your role and what you
delivered. Example:
```
Dev 1 [Database & Models]: DB wrapper, schema init, User/Account/Transaction CRUD
```

**No code in the update log.**
The update log is prose + tables. Do not paste implementation code — link to
the source files instead.

**Update your log if you change existing code.**
If Dev 3 touches a Dev 1 model to fix a bug, Dev 3 adds a note at the bottom
of their own `dev3.md` explaining what they changed and why — they do not edit
`dev1.md`.

**If you depend on another dev's module, say which version / commit you tested against.**
This makes it easy to bisect bugs that appear when two modules are combined.

---

## Example Checklist Before Pushing

- [ ] `updatelog/devN.md` written with all 5 sections filled in
- [ ] Build passes with zero errors and zero warnings
- [ ] Smoke test or manual test output pasted in update log
- [ ] Commit message follows the format above
- [ ] No `.db` files, no `build/` directory, no `.claude/` directory committed
