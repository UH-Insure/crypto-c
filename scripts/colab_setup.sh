#!/usr/bin/env bash
set -euo pipefail

# --- LLVM / Clang 14 installation ---
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

# --- SAW + Cryptol installation (for verification scripts) ---
if ! command -v saw >/dev/null 2>&1; then
  echo "[*] Installing SAW + Cryptol..."
  SAW_URL="https://github.com/GaloisInc/saw-script/releases/download/v1.3/saw-1.3-ubuntu-20.04-X64-with-solvers.tar.gz"
  sudo apt-get update && sudo apt-get install -y z3 curl tar
  curl -fsSL "$SAW_URL" -o /tmp/saw.tar.gz
  sudo mkdir -p /opt/saw
  sudo tar -xf /tmp/saw.tar.gz -C /opt/saw --strip-components=1
  sudo ln -sf /opt/saw/bin/saw /usr/local/bin/saw
  sudo ln -sf /opt/saw/bin/cryptol /usr/local/bin/cryptol
  rm -f /tmp/saw.tar.gz
fi

# --- Make SAW accessible in new shells (important for Colab) ---
echo 'export PATH="/opt/saw/bin:$PATH"' >> ~/.bashrc
export PATH="/opt/saw/bin:$PATH"

# --- Build project and run tests ---
echo "[*] Building..."
make
echo "[*] Running binary..."
./build/crypto_demo || true
echo "[*] Running tests..."
make test
echo "[*] Done."
