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

## Time-scheduled encoding

`beeping-core` exposes three C-API entry points that turn a short payload
into a series of beeps spread across `duration` seconds. Each beep's payload
is `code + base32(round(timestamp_seconds))`, so a decoder can recover its
position within the schedule.

```c
#include <BeepingCoreLib_api.h>

void* core = BEEPING_Create();
BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core);

// 1) Query the required output buffer size (samples)
int32_t requiredSamples = BEEPING_GetScheduleBufferSize(/*duration=*/10.0f, core);

// 2) (Optional) Inspect the schedule timestamps in seconds
double timestamps[16];
int32_t count = 0;
BEEPING_ComputeBeepSchedule(/*duration*/ 10.0f, /*startTime*/ 0.0f,
                            /*interval*/ 2.3f, timestamps, 16, &count);
// count == 4, timestamps == {0.0, 2.3, 4.6, 6.9}

// 3) Render the audio
float* pcm = (float*)malloc(requiredSamples * sizeof(float));
int32_t written = 0;
BEEPING_EncodeWithSchedule("abcde", 5, /*type*/ 0, NULL, 0,
                           /*duration*/ 10.0f, /*startTime*/ 0.0f,
                           /*interval*/ 2.3f, /*beepGainDb*/ -3.0f,
                           pcm, requiredSamples, &written, core);

// pcm now holds `written` mono float samples at 44100 Hz, ready to play
// or to dump into a WAV.

free(pcm);
BEEPING_Destroy(core);
```

Constraints: `duration` and `interval` must be `>= 2.3` (the minimum beep
window), `startTime >= 0`, `startTime + 2.3 <= duration`. `beepGainDb` is
clamped to `[-60, +12]`.

On the **decode side**, a single captured beep yields the full payload
(`code + base32(ts)`). Split it via the matching parser so SDK wrappers
don't reimplement the convention:

```c
// After BEEPING_DecodeAudioBuffer(...) signals -3 (word decoded):
char code[16] = {};
int32_t codeLen = 0;
int32_t tsSec = 0;
int32_t rc = BEEPING_GetDecodedScheduledPayload(
    code, sizeof(code), &codeLen, &tsSec, core);
if (rc > 0) {
  // code  == "abcde"
  // tsSec ==  4   (i.e. the captured beep is the one at t=4.6s, rounded)
}
```

`BEEPING_ParseScheduledPayload` is also exposed for parsing payloads
obtained from elsewhere (e.g. log lines, network frames).

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
