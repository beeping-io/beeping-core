# 🔐 Verifying beeping-core releases

Every published release of `beeping-core` ships with a full supply-chain
security bundle so you can cryptographically verify that each binary was
produced by our CI from a specific commit, and that its dependencies are
publicly auditable.

Each release includes:

| Artifact | Purpose |
|---|---|
| `beeping-core-<target>.tar.zst` | Compiled binary for the target platform |
| `beeping-core-<target>.tar.zst.cosign.bundle` | Cosign keyless bundle (cert + sig + Rekor entry) |
| `beeping-core-sbom.cdx.json` | CycloneDX Software Bill of Materials |
| `beeping-core-sbom.cdx.json.cosign.bundle` | Cosign keyless bundle for the SBOM |
| `beeping-core.intoto.jsonl` | SLSA L3 provenance (one file per release) |
| `SHA256SUMS.txt` | Integrity checksums for every artifact |

The three orthogonal verifications you should run:

| Check | What it proves | Tool |
|---|---|---|
| **SHA256** | File was not truncated or corrupted in transit | `shasum`, `sha256sum` |
| **Cosign keyless** | File was signed by our release workflow (OIDC, no long-lived keys) | `cosign` |
| **SLSA L3 provenance** | File was produced by our workflow at a specific commit (highest level of build integrity) | `slsa-verifier` |

---

## 1. Integrity check (SHA256)

```bash
# Download the artifact and the checksums
RELEASE=v1.0.0
ARTIFACT=beeping-core-macos-universal.tar.zst

curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/SHA256SUMS.txt

# Verify
shasum -a 256 -c SHA256SUMS.txt --ignore-missing
# Expected: beeping-core-macos-universal.tar.zst: OK
```

On Linux replace `shasum -a 256 -c` with `sha256sum -c`.

---

## 2. Cosign keyless signature verification

### Install cosign

```bash
# macOS
brew install cosign

# Linux (see https://docs.sigstore.dev/system_config/installation/)
curl -O -L https://github.com/sigstore/cosign/releases/latest/download/cosign-linux-amd64
chmod +x cosign-linux-amd64 && sudo mv cosign-linux-amd64 /usr/local/bin/cosign
```

### Verify a binary

```bash
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT.cosign.bundle

cosign verify-blob \
  --bundle $ARTIFACT.cosign.bundle \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  $ARTIFACT

# Expected: Verified OK
```

The bundle embeds the signing certificate and the Rekor transparency-log
entry, so this single file is everything a downstream consumer needs to
verify. There are no long-lived signing keys — cosign uses a short-lived
OIDC token from GitHub Actions.

### Verify the SBOM

Same flow, with the SBOM file:

```bash
cosign verify-blob \
  --bundle beeping-core-sbom.cdx.json.cosign.bundle \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  beeping-core-sbom.cdx.json
```

---

## 3. SLSA L3 provenance verification

### Install slsa-verifier

```bash
# macOS arm64
curl -LO https://github.com/slsa-framework/slsa-verifier/releases/latest/download/slsa-verifier-darwin-arm64
chmod +x slsa-verifier-darwin-arm64 && sudo mv slsa-verifier-darwin-arm64 /usr/local/bin/slsa-verifier

# Linux amd64
curl -LO https://github.com/slsa-framework/slsa-verifier/releases/latest/download/slsa-verifier-linux-amd64
chmod +x slsa-verifier-linux-amd64 && sudo mv slsa-verifier-linux-amd64 /usr/local/bin/slsa-verifier
```

> **Note**: use `slsa-verifier >= v2.7.0`. Older versions do not understand the
> Sigstore bundle v0.3 format produced by the latest SLSA generator.

### Verify an artifact

```bash
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/beeping-core.intoto.jsonl

slsa-verifier verify-artifact $ARTIFACT \
  --provenance-path beeping-core.intoto.jsonl \
  --source-uri github.com/beeping-io/beeping-core \
  --source-tag $RELEASE

# Expected:
# Verified build using builder "https://github.com/slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@refs/tags/v2.1.0"
#  at commit <sha>
# PASSED: SLSA verification passed
```

A successful verification tells you:

- The artifact was built by our known GitHub Actions reusable workflow
- From the exact source commit and tag of the release
- On an isolated SLSA-compliant runner
- Without tampering post-build

This covers SLSA Build Level 3 requirements.

---

## 4. Inspect the SBOM (CycloneDX)

The SBOM lists every dependency and tool used during the build. It is
machine-readable JSON conforming to [CycloneDX 1.6](https://cyclonedx.org/).

### With jq

```bash
# Count components
jq '.components | length' beeping-core-sbom.cdx.json

# List all components
jq -r '.components[] | "\(.name) \(.version // "?")"' beeping-core-sbom.cdx.json

# Check licenses
jq -r '.components[] | "\(.name): \(.licenses // "(none)")"' beeping-core-sbom.cdx.json
```

### With syft / grype

```bash
# Pretty table of components
syft sbom beeping-core-sbom.cdx.json -o table

# Scan for known vulnerabilities
grype sbom:beeping-core-sbom.cdx.json
```

---

## Full verification script

```bash
#!/usr/bin/env bash
set -euo pipefail
RELEASE="${1:-v1.0.0}"
ARTIFACT="${2:-beeping-core-macos-universal.tar.zst}"
BASE="https://github.com/beeping-io/beeping-core/releases/download/$RELEASE"

echo "→ Downloading artifact, bundle, checksums, provenance, SBOM"
curl -sLO "$BASE/$ARTIFACT"
curl -sLO "$BASE/$ARTIFACT.cosign.bundle"
curl -sLO "$BASE/SHA256SUMS.txt"
curl -sLO "$BASE/beeping-core.intoto.jsonl"
curl -sLO "$BASE/beeping-core-sbom.cdx.json"
curl -sLO "$BASE/beeping-core-sbom.cdx.json.cosign.bundle"

echo "→ 1/3 SHA256 integrity"
shasum -a 256 -c SHA256SUMS.txt --ignore-missing

echo "→ 2/3 Cosign signature"
cosign verify-blob \
  --bundle "$ARTIFACT.cosign.bundle" \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  "$ARTIFACT"

echo "→ 3/3 SLSA provenance"
slsa-verifier verify-artifact "$ARTIFACT" \
  --provenance-path beeping-core.intoto.jsonl \
  --source-uri github.com/beeping-io/beeping-core \
  --source-tag "$RELEASE"

echo "✅ All verifications passed for $ARTIFACT"
```

---

## 5. 🍎 iOS-specific: XCFramework structure verification

The iOS artifact `beeping-core-ios-xcframework.tar.zst` ships a single
`BeepingCore.xcframework` directory containing two slices:

| Slice | Architecture(s) | Use |
|---|---|---|
| `ios-arm64` | `arm64` | Real iPhones / iPads |
| `ios-arm64_x86_64-simulator` | `arm64` + `x86_64` (universal) | Simulator on Apple Silicon **and** legacy Intel Macs |

Verify locally with `lipo` (bundled with Xcode) and `plutil`:

```bash
RELEASE=v1.0.0
ARTIFACT=beeping-core-ios-xcframework.tar.zst
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT.cosign.bundle

# (Recommended) cosign verify first — same flow as section 2
cosign verify-blob \
  --bundle $ARTIFACT.cosign.bundle \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  $ARTIFACT

# Extract + inspect
mkdir -p extract && tar --zstd -xf $ARTIFACT -C extract
XCFW=extract/BeepingCore.xcframework

# Info.plist must be a valid plist
plutil -lint "$XCFW/Info.plist"

# Each slice must carry the expected architectures
lipo -archs "$XCFW/ios-arm64/libBeepingCore.a"
# → arm64
lipo -archs "$XCFW/ios-arm64_x86_64-simulator/libBeepingCore.a"
# → x86_64 arm64
```

If `lipo -archs` reports an unexpected architecture set (e.g. only
`arm64` on the simulator slice), you have a partial XCFramework —
Apple Silicon devs would still be fine but the package would not load
in the simulator on legacy Intel Macs. CI guards both invariants in two
places (the `ios` build job + the `ios-smoke` post-package job) so a
release with a malformed XCFramework cannot be published.

To consume the XCFramework: drop it into your Xcode project, link
`BeepingCore.xcframework` from the Frameworks build phase, and import
the C API headers (`BeepingCoreLib_api.h`) from a bridging header or
modulemap.

The shipped slices are built with `-DCMAKE_BUILD_TYPE=RelWithDebInfo`
(`-O2 -g -DNDEBUG`), so DWARF debug info is embedded in the static
archives. When you archive your app, Xcode picks up those symbols and
emits a `.dSYM` for `BeepingCore` alongside the one for your app —
crash reports from production users will be fully symbolicatable.
You can confirm DWARF presence with `dwarfdump` (bundled with Xcode):

```bash
DEV=BeepingCore.xcframework/ios-arm64/libBeepingCore.a
dwarfdump --debug-info "$DEV" | grep -c "TAG_compile_unit"
# expected: ≥ 5  (one DWARF compile unit per .o; a healthy slice has dozens)
```

---

## 6. 🤖 Android-specific: 16 KB page-size verification

Android 15+ enforces 16 KB memory pages on apps published after Nov 2025.
The Android NDK shared libraries shipped by `beeping-core`
(`beeping-core-android-{arm64-v8a,armeabi-v7a,x86_64}.tar.zst`) are linked
with `-Wl,-z,max-page-size=16384`, but you can re-confirm it locally with
`readelf` from `binutils`.

```bash
# 1. Download + extract a per-ABI tarball
ABI=arm64-v8a   # or armeabi-v7a, x86_64
ARTIFACT=beeping-core-android-$ABI.tar.zst
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT.cosign.bundle

# 2. (Recommended) verify cosign signature first — same flow as section 2
cosign verify-blob \
  --bundle $ARTIFACT.cosign.bundle \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  $ARTIFACT

# 3. Extract and inspect ELF program headers
mkdir -p extract && tar --zstd -xf $ARTIFACT -C extract
SO=$(find extract -name "libbeepingcore.so" | head -1)
readelf -W -l "$SO" | grep "^  LOAD"
```

Use `readelf -W` (wide) so each program header is printed on a single
line — the default output splits a LOAD record across two lines, which
hides the Align column.

Each `LOAD` segment must show alignment `>= 0x4000` (16 KB) in the last
column:

```text
  LOAD  0x000000 0x00000000 0x00000000 0x13a270 0x13a270 R E 0x4000   ✅
  LOAD  0x13a270 0x0013e270 0x0013e270 0x00a2e0 0x00ad90 RW  0x4000   ✅
  LOAD  0x144550 0x0014c550 0x0014c550 0x0002e8 0x002440 RW  0x4000   ✅
```

If you see `0x1000` (4 KB) instead, the binary will fail to load at runtime
on Android 15+ devices with the error:

```text
java.lang.UnsatisfiedLinkError: dlopen failed:
"libbeepingcore.so" program alignment (4096) cannot be smaller than
system page size (16384)
```

CI guards this in two places: an inline `readelf` check in the `android`
job (build-time) and a separate `android-smoke` job that re-validates the
*packaged* tarball post-upload, so a release with a broken alignment
cannot be published.

---

## Reporting issues

If any verification fails unexpectedly, do not run the binary. Open an issue at
[github.com/beeping-io/beeping-core/issues](https://github.com/beeping-io/beeping-core/issues)
with the command you ran and the full output.
