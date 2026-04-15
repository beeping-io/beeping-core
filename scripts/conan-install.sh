#!/usr/bin/env bash
# Install Conan dependencies for beeping-core.
#
# Usage:
#   ./scripts/conan-install.sh            # Debug build (default)
#   ./scripts/conan-install.sh Release    # Release build

set -euo pipefail

BUILD_TYPE="${1:-Debug}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if ! command -v conan >/dev/null 2>&1; then
  echo "❌ conan not found. Install it first: pipx install conan" >&2
  exit 1
fi

CONAN_VERSION="$(conan --version | awk '{print $NF}')"
CONAN_MAJOR="${CONAN_VERSION%%.*}"
if [[ "$CONAN_MAJOR" -lt 2 ]]; then
  echo "❌ Conan 2.x required, found $CONAN_VERSION" >&2
  exit 1
fi

case "$(uname -s)" in
  Darwin) PROFILE="$REPO_ROOT/profiles/macos" ;;
  Linux)  PROFILE="$REPO_ROOT/profiles/linux" ;;
  *) echo "❌ Unsupported OS: $(uname -s)" >&2; exit 1 ;;
esac

cd "$REPO_ROOT"
conan install . \
  --build=missing \
  --lockfile=conan.lock \
  -pr:h="$PROFILE" \
  -pr:b="$PROFILE" \
  -s build_type="$BUILD_TYPE"

echo "✅ Conan deps installed for build_type=$BUILD_TYPE"
