# beeping-core

C++20 library for encoding and decoding data over sound (Data Over Sound).

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

## License

[Apache-2.0](LICENSE)
