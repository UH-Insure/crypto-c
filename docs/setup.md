# Setup Instructions

This file describes how to build and run the Crypto-AR-C repository in different environments.

---

## 1. Docker/Podman Setup

The project includes a `Dockerfile` that sets up an Ubuntu 22.04 container with **Clang 14** and build tools.

### Build and Run

```bash
# For bash users
chmod +x scripts/setup.sh
./scripts/setup.sh

# For zsh users
chmod +x scripts/setup.zsh
./scripts/setup.zsh
```

These scripts will:

* Build a container image `crypto-c-dev:clang14`.
* Start a container named `crypto-c-container`.
* Mount the repository into `/work` inside the container.

### Inside the Container

Once inside, you can build and test:

```bash
make        # build library + demo binary
make run    # run the demo binary
make test   # build and run all tests
```

---

## 2. Google Colab Setup

Colab does not support Docker, but you can use the provided script:

First cell in a Colab notebook:

```python
!git clone https://github.com/plobethus/crypto-c.git
%cd crypto-c
!chmod +x scripts/colab_setup.sh
!bash scripts/colab_setup.sh
```

This will:

* Install `clang-14` and supporting tools.
* Set `cc` and `c++` alternatives to clang-14.
* Build the project (`make`).
* Run demo and tests.

---

## 4. Makefile Targets

```bash
make            # Build demo binary into ./build
make run        # Run the demo program
make test       # Build and run all tests
make clean      # Remove build artifacts
make format     # Auto-format code (requires clang-format-14)
```

---

## Notes

* **Compiler**: Clang-14 is enforced to keep builds reproducible.
* **Randomness**: Uses `/dev/urandom` inside Linux containers.
* **Dependencies**: Entire project is written in pure C, no OpenSSL required.
