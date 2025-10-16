FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# ---------- Base toolchain (Clang/LLVM 16) ----------
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates wget curl gnupg lsb-release software-properties-common \
  && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
  && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" \
        > /etc/apt/sources.list.d/llvm16.list \
  && apt-get update && apt-get install -y --no-install-recommends \
      clang-16 clang-tools-16 clang-format-16 lldb-16 lld-16 \
      libc++-16-dev libc++abi-16-dev \
      build-essential cmake git bash zsh \
  && rm -rf /var/lib/apt/lists/*

# Make clang-16 the default cc/c++
RUN update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-16    100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-16  100 && \
    update-alternatives --set cc  /usr/bin/clang-16 && \
    update-alternatives --set c++ /usr/bin/clang++-16

# ---------- SAW + Cryptol + Solvers ----------
# Z3/Yices are needed by SAW; SAW bundle includes Cryptol.
#   docker build --build-arg SAW_URL=https://github.com/GaloisInc/saw-script/releases/download/vX.Y/saw-...-linux.tar.gz .
ARG SAW_URL=
RUN apt-get update && apt-get install -y --no-install-recommends \
      z3 yices2 tar ca-certificates curl \
  && rm -rf /var/lib/apt/lists/* \
  && set -eux; \
     if [ -z "$SAW_URL" ]; then \
       SAW_URL="$(curl -s https://api.github.com/repos/GaloisInc/saw-script/releases/latest \
                  | grep browser_download_url | grep linux | cut -d '\"' -f 4 | head -n1)"; \
     fi; \
     echo "Downloading SAW from: $SAW_URL"; \
     curl -L "$SAW_URL" -o /tmp/saw.tar.gz; \
     mkdir -p /opt/saw && tar -xf /tmp/saw.tar.gz -C /opt/saw --strip-components=1; \
     ln -sf /opt/saw/bin/saw /usr/local/bin/saw; \
     ln -sf /opt/saw/bin/cryptol /usr/local/bin/cryptol; \
     rm -f /tmp/saw.tar.gz

# ---------- Workspace ----------
WORKDIR /work

# Copy manifests first (cache-friendly), then sources
COPY Makefile CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests

RUN cc --version && clang-format-16 --version && saw --version && cryptol --version && z3 --version && yices-smt2 --version

# Default shell
CMD ["/bin/bash"]