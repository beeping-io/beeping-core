# Installing beeping-core

> **Status**: macOS validated end-to-end (v0.1.0). Other platforms are built
> and shipped only after they pass their own end-to-end validation. No
> assumptions, one OS at a time.

| Platform | Status | Latest release |
|---|---|---|
| 🍎 macOS (universal arm64 + x86_64) | ✅ Validated | v0.1.0 |
| 🐧 Linux amd64 (glibc — Ubuntu 22+/24+, Debian 12+, Fedora 41+, Arch) | ✅ Validated | v0.2.0 |
| 🪟 Windows (x64) | ⏳ Coming | — |
| 🌐 WASM (browser) | ⏳ Coming | — |
| 🪟 Windows ARM64 | ⏳ Coming | — |
| 🥧 Raspberry Pi / Linux ARM64 | ⏳ Coming | — |
| 📱 iOS XCFramework | ⏳ Phase 9 | — |
| 🤖 Android NDK | ⏳ Phase 8 | — |
| 🏔️ Alpine Linux / musl | ❌ Not supported (glibc-only build) | — |

## 🍎 macOS (universal — arm64 + x86_64)

Runs natively on both Apple Silicon (M1/M2/M3/M4) and Intel Macs, no Rosetta
needed. The same binary is fat-packaged with both architectures.

### Download

Replace `v0.1.0` with the latest release tag:

```bash
TAG=v0.1.0
curl -LO "https://github.com/beeping-io/beeping-core/releases/download/${TAG}/beeping-core-macos-universal.tar.zst"
curl -LO "https://github.com/beeping-io/beeping-core/releases/download/${TAG}/SHA256SUMS.txt"
```

### Verify

```bash
shasum -a 256 -c SHA256SUMS.txt --ignore-missing
```

Expected: `beeping-core-macos-universal.tar.zst: OK`.

### Extract

```bash
tar --zstd -xf beeping-core-macos-universal.tar.zst
```

You now have a `bin/` and `lib/` + `include/` directory.

### Run the CLI

```bash
./bin/beeping-core --help
./bin/beeping-core --version
```

Verify it's truly universal:

```bash
lipo -info ./bin/beeping-core
# Output: Architectures in the fat file: ./bin/beeping-core are: x86_64 arm64
```

### Decode a WAV

```bash
./bin/beeping-core decode path/to/sound.wav
```

## 🐧 Linux amd64 (glibc — Ubuntu, Debian, Fedora, Arch)

Validated end-to-end on **Ubuntu 22.04 + 24.04**, **Debian 12**, **Fedora 41**, and **Arch Linux** (rolling). Built against Ubuntu LTS glibc; should work on any glibc 2.35+ distro.

### Download

```bash
TAG=v0.2.0
curl -LO "https://github.com/beeping-io/beeping-core/releases/download/${TAG}/beeping-core-linux-amd64.tar.zst"
curl -LO "https://github.com/beeping-io/beeping-core/releases/download/${TAG}/SHA256SUMS.txt"
```

### Verify

```bash
sha256sum -c SHA256SUMS.txt --ignore-missing
```

### Extract

```bash
# zstd may need installing on minimal images:
#   apt:    sudo apt-get install zstd
#   dnf:    sudo dnf install zstd
#   pacman: sudo pacman -S zstd

tar --zstd -xf beeping-core-linux-amd64.tar.zst
```

### Run the CLI

```bash
./bin/beeping-core --help
./bin/beeping-core --version
./bin/beeping-core decode path/to/sound.wav
```

### Not supported: Alpine / musl

Alpine Linux uses musl libc, not glibc — the binary won't run there. If demand surfaces, a separate `beeping-core-linux-amd64-musl.tar.zst` variant will be added in its own task.

## Other platforms

Each platform goes through its own validation task before release. Track
progress in the [project roadmap](../docs/ROADMAP.md).

## Building from source

Also supported — see the [Development guide](./DEVELOPMENT.md) (coming with
first Linux validation).
