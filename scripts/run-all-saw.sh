#!/usr/bin/env bash
set -euo pipefail

# --- helpers ---------------------------------------------------------------
have() { command -v "$1" >/dev/null 2>&1; }
as_root() { if have sudo; then sudo "$@"; else "$@"; fi }

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "[*] Ensuring llvm-link is available (llvm-14)…"
if ! have llvm-link; then
  if [ -x /usr/lib/llvm-14/bin/llvm-link ]; then
    as_root ln -sf /usr/lib/llvm-14/bin/llvm-link /usr/local/bin/llvm-link
  else
    if have apt-get; then
      as_root apt-get update -y
      as_root apt-get install -y llvm-14
      as_root ln -sf /usr/lib/llvm-14/bin/llvm-link /usr/local/bin/llvm-link
    else
      echo "[!] llvm-14 not found and no apt-get available. Please install llvm-14." >&2
      exit 1
    fi
  fi
fi
llvm-link --version 1>/dev/null || { echo "[!] llvm-link still missing"; exit 1; }

echo "[*] Building LLVM bitcode for SAW…"
make -C verification clean
make -C verification

echo "[*] Ensuring SAW is available…"
if ! have saw; then
  echo "[*] Installing portable SAW (v1.3)…"
  tmp=/tmp/saw.tgz
  curl -fsSL https://github.com/GaloisInc/saw-script/releases/download/v1.3/saw-1.3-ubuntu-20.04-X64-with-solvers.tar.gz -o "$tmp"
  as_root mkdir -p /opt/saw
  as_root tar -xf "$tmp" -C /opt/saw --strip-components=1
  as_root ln -sf /opt/saw/bin/saw /usr/local/bin/saw
  as_root ln -sf /opt/saw/bin/cryptol /usr/local/bin/cryptol
fi
saw --version

echo
echo "[*] Running SAW for each script in verification/saw"
echo

fail=0
for f in verification/saw/*.saw; do
  echo "==> saw $f"
  if ! saw "$f"; then
    echo "[!] SAW failed: $f"
    fail=1
  fi
done

if [ "$fail" -ne 0 ]; then
  echo
  echo "[!] One or more proofs failed."
  exit 1
fi

echo
echo "[*] All proofs succeeded."