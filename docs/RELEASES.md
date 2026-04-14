# 📦 Pre-built binaries

`beeping-core` publishes pre-built binaries for every release on
[GitHub Releases](https://github.com/beeping-io/beeping-core/releases).

## Supported targets

| Target | Archs | Artifact |
|---|---|---|
| 🍎 macOS | arm64 + x86_64 universal | `beeping-core-macos-universal.tar.zst` |
| 🐧 Linux | amd64 | `beeping-core-linux-amd64.tar.zst` |
| 🐧 Linux | arm64 | `beeping-core-linux-arm64.tar.zst` |
| 🤖 Android NDK | arm64-v8a | `beeping-core-android-arm64-v8a.tar.zst` |
| 🤖 Android NDK | armeabi-v7a | `beeping-core-android-armeabi-v7a.tar.zst` |
| 🤖 Android NDK | x86_64 | `beeping-core-android-x86_64.tar.zst` |
| 🍎 iOS | device + simulator | `beeping-core-ios-xcframework.tar.zst` |
| 🌐 WASM | Emscripten | `beeping-core-wasm.tar.zst` |

Each release also includes:
- `SHA256SUMS.txt` — integrity checksums
- `<artifact>.sig` — cosign keyless signature

## Verify integrity

```bash
# Download artifact + checksums
curl -LO https://github.com/beeping-io/beeping-core/releases/download/v1.0.0/beeping-core-macos-universal.tar.zst
curl -LO https://github.com/beeping-io/beeping-core/releases/download/v1.0.0/SHA256SUMS.txt

# Verify
shasum -a 256 -c SHA256SUMS.txt --ignore-missing
```

## Verify cosign signature

```bash
cosign verify-blob \
  --certificate-identity-regexp "https://github.com/beeping-io/beeping-core/.github/workflows/release.yml@.*" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  --signature beeping-core-macos-universal.tar.zst.sig \
  beeping-core-macos-universal.tar.zst
```

The signature is tied to the GitHub Actions workflow that built the
artifact — no long-lived keys, no key management required (keyless
signing via OIDC).

## Extract and use

```bash
# macOS / Linux
tar --zstd -xf beeping-core-macos-universal.tar.zst
# Now you have: lib/libBeepingCore.a, include/BeepingCoreLib_api.h, etc.

# iOS (XCFramework)
tar --zstd -xf beeping-core-ios-xcframework.tar.zst
# Now you have: BeepingCore.xcframework — drag into Xcode
```

## Triggering a release

```bash
# Tag and push
git tag v1.0.0
git push origin v1.0.0

# …or manually from the GitHub UI:
# Actions → Release → Run workflow → enter tag
```
