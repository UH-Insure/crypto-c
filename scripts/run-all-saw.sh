#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
pushd "$ROOT/verification" >/dev/null

# Build the combined bitcode the SAW files expect
make build

echo "[*] Running SAW for each script in verification/saw"
shopt -s nullglob
for f in saw/*.saw; do
  echo "==> saw $f"
  saw "$f"
done
echo "[*] Done."