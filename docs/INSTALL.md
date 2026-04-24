# Installing beeping-core

> **Status**: macOS validated end-to-end (v0.1.0). Other platforms are built
> and shipped only after they pass their own end-to-end validation. No
> assumptions, one OS at a time.

| Platform | Status | Latest release |
|---|---|---|
| 🍎 macOS (universal arm64 + x86_64) | ✅ Validated | v0.1.0 |
| 🐧 Linux amd64 (glibc — Ubuntu 22+/24+, Debian 12+, Fedora 41+, Arch) | ✅ Validated | v0.2.0 |
| 🪟 Windows 11 (x64) | ✅ Validated | v0.3.1 |
| 🪟 Windows 11 (ARM64) | ✅ Validated | v0.4.0 |
| 🌐 WASM (browser) | ⏳ Coming | — |
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

## 🪟 Windows (x64)

Validated end-to-end on **Windows 11**. Built with MSVC + **static CRT** (`/MT`) — no VC++ redistributable required. Should also run on Windows 10 1809+ but officially validated on Windows 11 only.

### Download

Replace `v0.3.1` with the latest release tag:

```powershell
$tag = "v0.3.1"
Invoke-WebRequest -Uri "https://github.com/beeping-io/beeping-core/releases/download/$tag/beeping-core-windows-x64.zip" -OutFile beeping-core-windows-x64.zip
Invoke-WebRequest -Uri "https://github.com/beeping-io/beeping-core/releases/download/$tag/SHA256SUMS.txt" -OutFile SHA256SUMS.txt
```

### Verify

```powershell
$expected = (Get-Content SHA256SUMS.txt | Select-String "beeping-core-windows-x64.zip").ToString().Split(" ")[0]
$actual = (Get-FileHash beeping-core-windows-x64.zip -Algorithm SHA256).Hash.ToLower()
if ($expected -eq $actual) { Write-Host "✅ OK" } else { Write-Error "❌ Mismatch" }
```

### Extract

```powershell
Expand-Archive -Path .\beeping-core-windows-x64.zip -DestinationPath beeping-core
```

### Run the CLI

```powershell
.\beeping-core\bin\beeping-core.exe --help
.\beeping-core\bin\beeping-core.exe --version
.\beeping-core\bin\beeping-core.exe decode path\to\sound.wav
```

If Windows Defender SmartScreen warns: right-click the file → Properties → check "Unblock" at the bottom → OK. This is because the binary isn't code-signed yet (Authenticode signing will come in a later phase).

## 🪟 Windows (ARM64)

Validated end-to-end on **Windows 11 ARM64**. Built with MSVC ARM64 target + **static CRT** (`/MT`). Native ARM64 — no x64 emulation overhead.

Target hardware: Surface Pro (ARM64), Copilot+ PCs (Snapdragon X Elite/Plus), Parallels/UTM running Windows 11 on Apple Silicon.

### Download

Replace `v0.4.0` with the latest release tag:

```powershell
$tag = "v0.4.0"
Invoke-WebRequest -Uri "https://github.com/beeping-io/beeping-core/releases/download/$tag/beeping-core-windows-arm64.zip" -OutFile beeping-core-windows-arm64.zip
Invoke-WebRequest -Uri "https://github.com/beeping-io/beeping-core/releases/download/$tag/SHA256SUMS.txt" -OutFile SHA256SUMS.txt
```

### Verify

```powershell
$expected = (Get-Content SHA256SUMS.txt | Select-String "beeping-core-windows-arm64.zip").ToString().Split(" ")[0]
$actual = (Get-FileHash beeping-core-windows-arm64.zip -Algorithm SHA256).Hash.ToLower()
if ($expected -eq $actual) { Write-Host "✅ OK" } else { Write-Error "❌ Mismatch" }
```

### Extract + run

```powershell
Expand-Archive -Path .\beeping-core-windows-arm64.zip -DestinationPath beeping-core
.\beeping-core\bin\beeping-core.exe --help
.\beeping-core\bin\beeping-core.exe --version
.\beeping-core\bin\beeping-core.exe decode path\to\sound.wav
```

If your ARM64 Windows is running an x64 build of `beeping-core` under Prism emulation, decoding still works but isn't native. Prefer the ARM64 build for best battery + performance.

## Other platforms

Each platform goes through its own validation task before release. Track
progress in the [project roadmap](../docs/ROADMAP.md).

## Building from source

Also supported — see the [Development guide](./DEVELOPMENT.md) (coming with
first Linux validation).
