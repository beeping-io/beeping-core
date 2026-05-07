# 🔐 Verifying beeping-core releases

Every published release of `beeping-core` ships with a full supply-chain
security bundle so you can cryptographically verify that each binary was
produced by our CI from a specific commit, and that its dependencies are
publicly auditable.

Each release includes:

| Artifact | Purpose |
|---|---|
| `beeping-core-<target>.tar.zst` | Compiled binary for the target platform |
| `beeping-core-<target>.tar.zst.sig` | Cosign keyless signature |
| `beeping-core-sbom.cdx.json` | CycloneDX Software Bill of Materials |
| `beeping-core-sbom.cdx.json.sig` | Cosign signature for the SBOM |
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
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT.sig

cosign verify-blob \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  --signature $ARTIFACT.sig \
  $ARTIFACT

# Expected: Verified OK
```

The signature is tied to the exact GitHub Actions workflow that produced
the binary. There are no long-lived signing keys — cosign uses a short-lived
OIDC token from GitHub Actions.

### Verify the SBOM

Same flow, with the SBOM file:

```bash
cosign verify-blob \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  --signature beeping-core-sbom.cdx.json.sig \
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

echo "→ Downloading artifact, signature, checksums, provenance, SBOM"
curl -sLO "$BASE/$ARTIFACT"
curl -sLO "$BASE/$ARTIFACT.sig"
curl -sLO "$BASE/SHA256SUMS.txt"
curl -sLO "$BASE/beeping-core.intoto.jsonl"
curl -sLO "$BASE/beeping-core-sbom.cdx.json"
curl -sLO "$BASE/beeping-core-sbom.cdx.json.sig"

echo "→ 1/3 SHA256 integrity"
shasum -a 256 -c SHA256SUMS.txt --ignore-missing

echo "→ 2/3 Cosign signature"
cosign verify-blob \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  --signature "$ARTIFACT.sig" \
  "$ARTIFACT"

echo "→ 3/3 SLSA provenance"
slsa-verifier verify-artifact "$ARTIFACT" \
  --provenance-path beeping-core.intoto.jsonl \
  --source-uri github.com/beeping-io/beeping-core \
  --source-tag "$RELEASE"

echo "✅ All verifications passed for $ARTIFACT"
```

---

## 5. 🤖 Android-specific: 16 KB page-size verification

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
curl -LO https://github.com/beeping-io/beeping-core/releases/download/$RELEASE/$ARTIFACT.sig

# 2. (Recommended) verify cosign signature first — same flow as section 2
cosign verify-blob \
  --certificate-identity-regexp "^https://github.com/beeping-io/beeping-core/\\.github/workflows/release\\.yml@" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  --signature $ARTIFACT.sig \
  $ARTIFACT

# 3. Extract and inspect ELF program headers
mkdir -p extract && tar --zstd -xf $ARTIFACT -C extract
SO=$(find extract -name "libbeepingcore.so" | head -1)
readelf -l "$SO" | grep "^  LOAD"
```

Each `LOAD` segment must show alignment `>= 0x4000` (16 KB):

```
  LOAD           0x000000 ... 0x004000 R   0x4000   ✅ 16 KB OK
  LOAD           0x004000 ... 0x004000 R E 0x4000   ✅
  LOAD           0x010000 ... 0x004000 RW  0x4000   ✅
```

If you see `0x1000` (4 KB) instead, the binary will fail to load at runtime
on Android 15+ devices with the error:

```
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
