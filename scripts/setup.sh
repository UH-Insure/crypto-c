#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="crypto-c-dev:clang14"
CONTAINER_NAME="crypto-c-container"


if docker info 2>/dev/null | grep -qi podman; then
  VOL_SUFFIX=":Z"
else
  VOL_SUFFIX=""
fi

echo "[*] Building image ${IMAGE_NAME}..."
docker build -t "${IMAGE_NAME}" .

echo "[*] Starting container ${CONTAINER_NAME}..."
docker run --rm -it \
  --name "${CONTAINER_NAME}" \
  -v "$(pwd)":/work${VOL_SUFFIX} \
  -w /work \
  "${IMAGE_NAME}"
