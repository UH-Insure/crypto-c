# Ubuntu 22.04 + Clang 16 + SAW/Cryptol (pinned)
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

# --- Toolchain: Clang/LLVM 16 ---
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

# Make clang-16 default cc/c++
RUN update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-16    100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-16  100 && \
    update-alternatives --set cc  /usr/bin/clang-16 && \
    update-alternatives --set c++ /usr/bin/clang++-16

# --- SAW + Cryptol + Z3 (pin SAW bundle) ---
ARG SAW_URL="https://github.com/GaloisInc/saw-script/releases/download/v1.3/saw-1.3-ubuntu-20.04-X64-with-solvers.tar.gz"
RUN apt-get update && apt-get install -y --no-install-recommends \
      z3 tar ca-certificates curl \
  && rm -rf /var/lib/apt/lists/* \
  && echo "Downloading SAW from: $SAW_URL" \
  && curl -fsSL "$SAW_URL" -o /tmp/saw.tar.gz \
  && mkdir -p /opt/saw \
  && tar -xf /tmp/saw.tar.gz -C /opt/saw --strip-components=1 \
  && test -x /opt/saw/bin/saw && test -x /opt/saw/bin/cryptol \
  && ln -sf /opt/saw/bin/saw     /usr/local/bin/saw \
  && ln -sf /opt/saw/bin/cryptol /usr/local/bin/cryptol \
  && rm -f /tmp/saw.tar.gz
ENV PATH="/opt/saw/bin:${PATH}"

# --- Workspace ---
WORKDIR /work
COPY Makefile CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests

# Version checks (non-fatal)
RUN cc --version || true \
 && clang-format-16 --version || true \
 && saw --version || true \
 && cryptol --version || true \
 && z3 --version || true

CMD ["/bin/bash"]