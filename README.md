# beeping-core

C++20 library for encoding and decoding data over sound (Data Over Sound).

<!-- Identity -->
![License](https://img.shields.io/badge/License-Apache_2.0-blue)
![status](https://img.shields.io/badge/status-early_development-orange)
![platform](https://img.shields.io/badge/platform-Beeping-purple)
![conventional commits](https://img.shields.io/badge/conventional_commits-1.0.0-yellow)

<!-- Tech stack -->
![C++](https://img.shields.io/badge/C++-20-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.25+-064F8C?logo=cmake&logoColor=white)
![Conan](https://img.shields.io/badge/Conan-2.x-6699CB?logo=conan&logoColor=white)

<!-- CI / Quality -->
[![Build & Test](https://github.com/beeping-io/beeping-core/actions/workflows/ci.yml/badge.svg)](https://github.com/beeping-io/beeping-core/actions/workflows/ci.yml)
[![ThreadSanitizer](https://github.com/beeping-io/beeping-core/actions/workflows/tsan.yml/badge.svg)](https://github.com/beeping-io/beeping-core/actions/workflows/tsan.yml)
[![Static Analysis](https://github.com/beeping-io/beeping-core/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/beeping-io/beeping-core/actions/workflows/static-analysis.yml)
[![Docs](https://github.com/beeping-io/beeping-core/actions/workflows/docs.yml/badge.svg)](https://core-docs.beeping.io)

## Requirements

- C++20 compiler (Apple Clang 15+, GCC 13+, Clang 16+)
- CMake 3.25+
- [Conan 2.x](https://conan.io/) (`pipx install conan`)

## Build

Dependencies are managed by **Conan 2.x in manifest mode** with a committed
`conan.lock` for fully deterministic builds.

```bash
./scripts/conan-install.sh        # installs deps for Debug (default)
cmake --preset default
cmake --build --preset default
ctest --preset default
```

For a Release build:

```bash
./scripts/conan-install.sh Release
cmake --preset release
cmake --build --preset release
```

### Presets

| Preset | Description |
|---|---|
| `default` | Debug build with tests |
| `ci` | CI build (Debug + tests + `compile_commands.json`) |
| `release` | Release build, no tests |

Each preset wires the Conan-generated toolchain at
`build/<BuildType>/generators/conan_toolchain.cmake`, so `conan install` must
be run before `cmake --preset` for the matching build type.

### Conan profiles

Project-managed profiles live in [`profiles/`](profiles/) (`macos`, `linux`).
The install script auto-selects by `uname -s`. Pinning the profiles in-repo
keeps CI and local builds bit-for-bit reproducible regardless of the host
default profile.

## Releases & verification

Pre-built binaries for macOS, Linux, Android, iOS and WASM are published on
[GitHub Releases](https://github.com/beeping-io/beeping-core/releases) with
full supply-chain security:

- **Cosign keyless signatures** (no long-lived keys)
- **CycloneDX SBOM** (dependency inventory)
- **SLSA L3 provenance** (build attestation)

See [`docs/verifying-releases.md`](docs/verifying-releases.md) for full
verification instructions.

## License

[Apache-2.0](LICENSE)
