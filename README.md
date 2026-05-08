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

<!-- Platform validation status -->
![macOS](https://img.shields.io/badge/🍎_macOS-validated-brightgreen)
![Linux](https://img.shields.io/badge/🐧_Linux-validated-brightgreen)
![Windows x64](https://img.shields.io/badge/🪟_Windows_x64-validated-brightgreen)
![Windows ARM64](https://img.shields.io/badge/🪟_Windows_ARM64-validated-brightgreen)
![WASM](https://img.shields.io/badge/🌐_WASM-validated-brightgreen)
![Raspberry Pi](https://img.shields.io/badge/🥧_Raspberry_Pi-ready-brightgreen)
![iOS](https://img.shields.io/badge/📱_iOS-Phase_9-lightgrey)
![Android](https://img.shields.io/badge/🤖_Android-Phase_8-lightgrey)

> **Cross-platform strategy**: each OS is validated end-to-end (download
> tarball → run CLI → round-trip test) before being promoted to the release.
> "Validated" = we've actually used it on that platform and it works.
> See [docs/INSTALL.md](docs/INSTALL.md) for installation per platform.

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

Project-managed profiles live in [`profiles/`](profiles/):
`macos`, `linux`, `windows-x64`, `windows-arm64`.

- On macOS/Linux the install script (`scripts/conan-install.sh`) picks the
  profile by `uname -s`.
- On Windows use `scripts/conan-install.ps1` (PowerShell), which picks
  `windows-x64` or `windows-arm64` based on `$env:PROCESSOR_ARCHITECTURE`.

Pinning the profiles in-repo keeps CI and local builds bit-for-bit
reproducible regardless of the host default profile.

## CLI — decode a WAV

The build produces a `beeping-core` binary (at
`build/Debug/cli/beeping-core`) that can decode a WAV file produced by
the Beeping platform:

```bash
./build/Debug/cli/beeping-core version
# BeepingCoreLib version 1.0.0 [...]

./build/Debug/cli/beeping-core decode audio.wav
# payload-string

./build/Debug/cli/beeping-core decode -m 2 audio.wav   # force audible
./build/Debug/cli/beeping-core decode -m 3 audio.wav   # force inaudible
# default is -m 5 (try both)
```

Exit codes: `0` = decoded (payload on stdout), `1` = no beep found,
`2` = bad input (missing file, unreadable WAV, etc.).

Supports WAV PCM int16 (format 1) and IEEE float 32-bit (format 3),
mono or stereo (downmixed to mono).

Full docs: [`docs/cli.md`](docs/cli.md).

## Releases & verification

Pre-built binaries for macOS, Linux, Android, iOS and WASM are published on
[GitHub Releases](https://github.com/beeping-io/beeping-core/releases) with
full supply-chain security:

- **Cosign keyless signatures** (no long-lived keys)
- **CycloneDX SBOM** (dependency inventory)
- **SLSA L3 provenance** (build attestation)

See [`docs/verifying-releases.md`](docs/verifying-releases.md) for full
verification instructions.

## Using beeping-core in your project

Two supported consumption paths via Conan 2.x:

- **ConanCenter** (recipe pending approval) — `conan install beeping-core/0.0.0`
- **GitHub Release fallback** (available now) — pre-built binaries or local
  recipe export from a git tag

Full guide with copy-paste examples: [`docs/conan.md`](docs/conan.md).

## License

[Apache-2.0](LICENSE)
