# ===============================
#  SAW + Cryptol + Full SMT Solver Suite
# ===============================

# Pull SAW and Cryptol images
FROM ghcr.io/galoisinc/saw:nightly AS sawsrc
FROM ghcr.io/galoisinc/cryptol:3.2.0 AS cryptolsrc
#RUN docker pull ghcr.io/galoisinc/cryptol-remote-api:2.13.0

# Base image: Ubuntu 22.04
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

# --- Install Clang/LLVM 16 and build tools ---
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates wget curl gnupg lsb-release software-properties-common \
  && curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /usr/share/keyrings/llvm.gpg \
  && echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" \
       > /etc/apt/sources.list.d/llvm16.list \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
       clang-16 clang-tools-16 clang-format-16 lldb-16 lld-16 \
       llvm-16 libc++-16-dev libc++abi-16-dev \
       build-essential cmake git z3 bash zsh \
  && rm -rf /var/lib/apt/lists/*

# Make clang-16 default
RUN update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-16  100 \
 && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-16 100 \
 && update-alternatives --set cc  /usr/bin/clang-16 \
 && update-alternatives --set c++ /usr/bin/clang++-16

# --- Install additional SMT solvers ---
RUN apt-get update && apt-get install -y --no-install-recommends \
      libgmp-dev libffi-dev python3 libreadline-dev unzip wget gnupg \
  && mkdir -p /opt/solvers && cd /opt/solvers \
  # ABC \
  && git clone https://github.com/berkeley-abc/abc.git \
  && cd abc && make -j$(nproc) && cp abc /usr/local/bin/ && cd .. \
  # Boolector \
  && git clone https://github.com/Boolector/boolector.git \
  && cd boolector && ./contrib/setup-lingeling.sh && ./contrib/setup-btor2tools.sh \
  && ./configure.sh && cd build && make -j$(nproc) install && cd ../.. \
  # CVC4 \
  && wget --no-verbose --user-agent="Mozilla/5.0" \
       https://cvc4.cs.stanford.edu/downloads/builds/x86_64-linux-opt/cvc4-1.8-x86_64-linux-opt \
  && chmod +x cvc4-1.8-x86_64-linux-opt \
  && mv cvc4-1.8-x86_64-linux-opt /usr/local/bin/cvc4 \
  # CVC5 \
  && wget --no-verbose --user-agent="Mozilla/5.0" \
       https://github.com/cvc5/cvc5/releases/download/cvc5-1.3.1/cvc5-Linux-x86_64-static.zip \
  && unzip cvc5-Linux-x86_64-static.zip \
  && cp cvc5-Linux-x86_64-static/bin/cvc5 /usr/local/bin/cvc5 \
  && rm -rf cvc5-Linux-x86_64-static* \
  # MathSAT (version 5.6.12) \
  && wget --no-verbose --user-agent="Mozilla/5.0" \
       https://mathsat.fbk.eu/release/mathsat-5.6.12-linux-x86_64.tar.gz -O mathsat.tar.gz \
  && tar -xzf mathsat.tar.gz \
  && cp mathsat-*/bin/mathsat /usr/local/bin/mathsat \
  && rm -rf mathsat* \
  # Yices 2.6.4 \
  && wget --no-verbose --user-agent="Mozilla/5.0" \
       https://yices.csl.sri.com/releases/2.6.4/yices-2.6.4-x86_64-pc-linux-gnu.tar.gz -O yices.tar.gz \
  && tar -xzf yices.tar.gz \
  && cp yices-*/bin/yices /usr/local/bin/yices \
  && rm -rf yices* \
  && rm -rf /var/lib/apt/lists/*

# Verify solver installation
RUN echo "\n=== Solvers Installed ===" && \
    which abc  || true && \
    which boolector || true && \
    which cvc4 || true && \
    which cvc5 || true && \
    which mathsat || true && \
    which yices || true && \
    which z3 || true && \
    echo "=== All solvers available ==="

# --- Copy SAW and Cryptol binaries ---
COPY --from=sawsrc /usr/local /usr/local
COPY --from=cryptolsrc /usr/local /usr/local
ENV PATH="/usr/local/bin:${PATH}"

# --- Set Cryptol path ---
ENV CRYPTOLPATH=/work/external/cryptol-specs

# --- Workspace setup ---
WORKDIR /work
COPY Makefile CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests

# --- Clone Cryptol specs repository ---
RUN mkdir -p /work/external && \
    git clone https://github.com/GaloisInc/cryptol-specs.git /work/external/cryptol-specs

# --- Optional debugging tools ---
RUN apt-get update && apt-get install -y --no-install-recommends vim less && \
    rm -rf /var/lib/apt/lists/*

# --- Version sanity checks ---
RUN echo "\n=== Toolchain Check ===" && \
    cc --version || true && \
    clang-format-16 --version || true && \
    saw --version || true && \
    cryptol --version || true && \
    z3 --version || true && \
    echo "=== All tools available ==="

# --- Default command ---
CMD ["/bin/bash"]