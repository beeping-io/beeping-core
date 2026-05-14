# 📦 Using beeping-core with Conan

`beeping-core` ships as a Conan 2.x package. There are **two supported ways**
to consume it, depending on whether the ConanCenter approval has landed.

## Quick reference

| Method | Availability | Use when |
|---|---|---|
| **1. ConanCenter** | 🚧 Pending approval — see [PR status](#status) | You want the standard, reviewed recipe |
| **2. GitHub Release fallback** | ✅ Available now | You need beeping-core today, can't wait for ConanCenter |

Both methods produce the **same binary** and the **same CMake target**
(`BeepingCore::BeepingCore`). Switching between them is a one-line change in
your `conanfile.txt` or `conanfile.py`.

---

## Method 1 — ConanCenter (recommended once approved)

### Status

The ConanCenter recipe is currently in review. Track progress at:

- PR: <https://github.com/conan-io/conan-center-index/pulls?q=beeping-core>
- This page is updated as soon as the recipe is merged.

### Consume from `conanfile.txt`

```ini
[requires]
beeping-core/0.0.0

[generators]
CMakeDeps
CMakeToolchain
```

### Consume from `conanfile.py`

```python
from conan import ConanFile

class MyApp(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("beeping-core/0.0.0")
```

### Install + build

```bash
conan install . --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
```

---

## Method 2 — GitHub Release fallback (available now)

Until the ConanCenter recipe is merged, you can consume `beeping-core`
directly from its GitHub Releases. Two sub-options:

### 2a — Pre-built binaries (fastest)

Each tagged release publishes ready-to-link static libraries for macOS
(universal), Linux (amd64 + arm64), Android (3 ABIs), iOS (XCFramework)
and WASM. See [docs/verifying-releases.md](verifying-releases.md) for
download + cryptographic verification.

```bash
RELEASE=v0.0.0
ARTIFACT=beeping-core-macos-universal.tar.zst

curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT
tar --zstd -xf $ARTIFACT -C /usr/local
```

Then in your `CMakeLists.txt`:

```cmake
find_package(BeepingCore REQUIRED CONFIG)
target_link_libraries(my_app PRIVATE BeepingCore::BeepingCore)
```

This bypasses Conan entirely. Use it when you don't need dependency
resolution and trust the verified binary.

### 2b — Build from source via Conan + git

If you want the Conan workflow (lockfiles, multi-platform builds,
transitive deps), point the recipe at a git tag:

`conanfile.py` of your project:

```python
from conan import ConanFile

class MyApp(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        # Reference resolved from local recipe (option 1) or git (option 2)
        self.requires("beeping-core/0.0.0@beeping/stable")
```

Export the recipe locally from a clone of beeping-core:

```bash
git clone --depth=1 --branch v0.0.0 https://github.com/beeping-io/beeping-core.git
cd beeping-core
conan export recipe/all --version=0.0.0 --user=beeping --channel=stable
```

Now `conan install .` in your project resolves
`beeping-core/0.0.0@beeping/stable` from your local cache, builds it once,
and reuses the binary across builds.

### 2c — Custom Conan remote (planned)

A self-hosted Conan remote at `https://conan.beeping.io` is on the
roadmap. When live:

```bash
conan remote add beeping-gh https://conan.beeping.io
conan install . --build=missing -r=beeping-gh
```

Until then, use 2a or 2b.

---

## CMake integration

Regardless of which method you used, the CMake usage is identical:

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_app LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(BeepingCore REQUIRED CONFIG)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE BeepingCore::BeepingCore)
```

The target `BeepingCore::BeepingCore` carries:

- include directories (public API headers under `BeepingCoreLib_api.h`,
  `BeepingConfig.h`, `Encoder.h`, `Decoder.h`, …)
- the static (or shared, if `-o shared=True`) library
- transitive `spdlog` dependency
- C++20 requirement

---

## Migration: fallback → ConanCenter

When the ConanCenter recipe is merged, switch is a one-line change:

```diff
-self.requires("beeping-core/0.0.0@beeping/stable")
+self.requires("beeping-core/0.0.0")
```

Or in `conanfile.txt`:

```diff
-beeping-core/0.0.0@beeping/stable
+beeping-core/0.0.0
```

No code changes required. Same headers, same target, same ABI.

---

## Verification

All published binaries are signed with **cosign keyless** and carry
**SLSA L3 provenance**. Always verify before consuming:

```bash
cosign verify-blob \
  --bundle beeping-core-macos-universal.tar.zst.cosign.bundle \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  beeping-core-macos-universal.tar.zst
```

Full instructions: [docs/verifying-releases.md](verifying-releases.md).

---

## FAQ

### Why two consumption methods?

ConanCenter PRs typically take 1–4 weeks of review and CI iteration. The
Beeping ecosystem (wrappers, SDKs, apps) cannot wait that long, so we ship
a fallback path on day one. The fallback is **not a workaround** — it's
identical in output and supported indefinitely.

### Will the API change between methods?

No. Both methods build from the same source tag and produce the same ABI.
The only difference is **how the recipe gets resolved** (ConanCenter
remote vs local export vs git clone).

### How do I know when ConanCenter is live?

- Watch the [release notes](https://github.com/beeping-io/beeping-core/releases)
  for a `chore: ConanCenter recipe approved` entry
- Or check `conan search beeping-core -r=conancenter`
- Or look for the ConanCenter badge in the repo README turning green

### Does the fallback ship with the same security guarantees?

Yes. Pre-built binaries are signed with cosign and carry SLSA L3
provenance; source builds via git use the exact tagged commit. The CycloneDX
SBOM is also published per release.

### Can I pin a specific commit instead of a tag?

Yes, with method 2b. Replace `--branch v0.0.0` in the `git clone` with
`--branch <commit-sha>` and adjust the version string. Note that
ConanCenter recipes only accept tagged versions.

---

## Reporting issues

If `conan install` or `conan create` fails for you with either method,
open an issue at
<https://github.com/beeping-io/beeping-core/issues> with:

- The exact command run
- Full output (Conan version, profile, error)
- Your OS / compiler / arch
