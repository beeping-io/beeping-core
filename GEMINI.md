# 🤖 GEMINI.md — `beeping-core`

> **Component-Level Instructions for Antigravity CLI (`agy`).**  
> Location: `/Users/fredi/Workspace/Beeping/beeping-core/GEMINI.md`.  
> Hierarchy: Inherits Global Methodology (`~/.gemini/config/rules/methodology.md`) and Workspace Rules (`../GEMINI.md`).

---

## 🎯 Role & Technology Stack

- **Purpose**: Central Data Over Sound DSP engine
- **Tech Stack**: `C++20 · CMake 3.25+ · Conan 2.x · Emscripten (WASM)`
- **Direct Consumers**: iOS, Android, CLI, Node, Python, Beepbox, Web
- **Ecosystem Note**: Core algorithms for acoustic modulation/demodulation. Never trust BEEPING_GetVersion(). Wire cable format is 9 chars (key[5] + timestamp[4]).

---

## 🛠️ Canonical Developer Commands

### 1. Build

```bash
conan install . --output-folder=build --build=missing -s build_type=Debug
cmake --preset default
cmake --build --preset default
```

### 2. Run Tests

```bash
ctest --preset default --output-on-failure
```

### 3. Lint & Format

```bash
# Lint checks
clang-format --dry-run --Werror src/*.cpp include/*.h

# Format code
clang-format -i src/*.cpp include/*.h
```

---

## 🚦 Quality Gates & Conventions

1. **Zero Tolerance Quality Gate**:
   - **0 test failures**
   - **0 lint warnings / violations**
   - Code formatting verified before reporting any task done.
2. **Lefthook Enforced**:
   - Pre-commit checks (whitespace, json/yaml lint, gitleaks).
   - Commit-msg Conventional Commits: `feat(scope): BEE-XXXX subject`.
   - Pre-push branch protection (blocks direct push to `develop` or `main`).
3. **Branching Model**:
   - Base branch: `develop`
   - Feature branch: `feat/<task-id>-<slug>`
   - Fix branch: `fix/<task-id>-<slug>`
   - Spike / Discovery branch: `spike/<task-id>-<slug>`
4. **Ecosystem Impact**:
   - If modifying APIs consumed by downstream packages, the `## 🔗 Ecosystem Impact` table must be declared and reviewed.
