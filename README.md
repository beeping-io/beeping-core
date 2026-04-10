# beeping-core

C++20 library for encoding and decoding data over sound (Data Over Sound).

## Build

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

### Presets

| Preset | Description |
|---|---|
| `default` | Debug build with tests |
| `release` | Release build, no tests |
| `ci` | CI build (debug + tests + compile_commands.json) |

## Requirements

- C++20 compiler (Clang 15+, GCC 12+, MSVC 2022+)
- CMake 3.25+

## License

[Apache-2.0](LICENSE)
