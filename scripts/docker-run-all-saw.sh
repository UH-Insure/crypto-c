#!/usr/bin/env bash
set -euo pipefail

IMG="${1:-crypto-c-dev:clang14}"   
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

INSIDE='
  set -euo pipefail
  cd /work/verification
  make build
  echo "[*] Running SAW scripts..."
  shopt -s nullglob
  for f in saw/*.saw; do
    echo "==> saw $f"
    saw "$f"
  done
'


docker run --rm -it \
  -v "$ROOT":/work \
  -w /work \
  '"$IMG"' \
  bash -lc "$INSIDE"