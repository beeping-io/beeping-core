#!/usr/bin/env bash
# Build and run beeping-core under ThreadSanitizer.
#
# Output: TSan reports go to stdout/stderr; pipe to a file if you want to
# attach the report to an audit (see BEE-21).
#
# Usage:
#   ./scripts/run-tsan.sh                  # full multi-threaded run
#   ./scripts/run-tsan.sh 2>&1 | tee tsan-report.log

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$REPO_ROOT"

"$SCRIPT_DIR/conan-install.sh" Debug

cmake --preset tsan
cmake --build --preset tsan

# Don't fail-fast: TSan output is the audit data, the harness exit code
# is informational.
ctest --preset tsan || true
