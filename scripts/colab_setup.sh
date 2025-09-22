#!/usr/bin/env bash
set -euo pipefail

if ! command -v clang-14 >/dev/null 2>&1; then
  echo "[*] Installing clang-14..."
  if sudo apt-get update && sudo apt-get install -y clang-14 clang-tools-14 lld-14 lldb-14; then
    :
  else
    echo "[*] Fallback: using LLVM install script"
    curl -fsSL https://apt.llvm.org/llvm.sh -o /tmp/llvm.sh
    chmod +x /tmp/llvm.sh
    sudo /tmp/llvm.sh 14
    sudo apt-get install -y lld-14 lldb-14 || true
  fi
fi

sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-14 100 || true
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-14 100 || true
sudo update-alternatives --set cc /usr/bin/clang-14 || true
sudo update-alternatives --set c++ /usr/bin/clang++-14 || true

echo "[*] Building..."
make
echo "[*] Running binary..."
./build/crypto_demo || true
echo "[*] Running tests..."
make test
echo "[*] Done."
