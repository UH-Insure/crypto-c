# Use Ubuntu 22.04
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Base tooling + add LLVM apt repo (jammy → clang-16)
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates wget gnupg lsb-release software-properties-common \
  && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
  && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" \
        > /etc/apt/sources.list.d/llvm16.list \
  && apt-get update && apt-get install -y --no-install-recommends \
      clang-16 clang-tools-16 lldb-16 lld-16 \
      libc++-16-dev libc++abi-16-dev \
      build-essential cmake git bash zsh \
  && rm -rf /var/lib/apt/lists/*

# Set clang-16/clang++-16 as default cc/c++
RUN update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-16    100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-16  100 && \
    update-alternatives --set cc  /usr/bin/clang-16 && \
    update-alternatives --set c++ /usr/bin/clang++-16

# Create workdir
WORKDIR /work

# Copy manifests first (cache-friendly), then sources
COPY Makefile CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests

# Default shell
CMD ["/bin/bash"]