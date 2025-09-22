# Use Ubuntu 22.04 
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install clang-14 and build tooling (no OpenSSL needed)
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang-14 clang-tools-14 lldb-14 lld-14 \
    build-essential cmake git bash zsh ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# Set clang-14 as default cc/c++
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-14 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-14 100 && \
    update-alternatives --set cc /usr/bin/clang-14 && \
    update-alternatives --set c++ /usr/bin/clang++-14

# Create workdir
WORKDIR /work

# Copy sources (CMakeLists.txt is optional, but we keep it for IDEs)
COPY Makefile CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests

# Default: open a shell; your scripts will `docker run -v ... /work`
CMD ["/bin/bash"]
