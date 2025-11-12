#!/usr/bin/env bash
set -euo pipefail

# Helper wrapper to build or upload ESP32 firmware using PlatformIO inside this container.
# Usage examples (run from project root, after docker build -t iiot-esp32 .):
#   Build:
#     docker run --rm -v "$PWD":/workspace -w /workspace iiot-esp32
#   Upload (Linux):
#     docker run --rm --device=/dev/ttyUSB0 -e ACTION=upload -e UPLOAD_PORT=/dev/ttyUSB0 -v "$PWD":/workspace -w /workspace iiot-esp32
#   Upload (macOS):
#     docker run --rm --device=/dev/cu.SLAB_USBtoUART -e ACTION=upload -e UPLOAD_PORT=/dev/cu.SLAB_USBtoUART -v "$PWD":/workspace -w /workspace iiot-esp32
#
# Environment variables:
#   ACTION       build | upload | monitor (default: build)
#   ENV_NAME     PlatformIO environment to use (default: esp32vn-iot-uno)
#   UPLOAD_PORT  Serial device path (e.g., /dev/ttyUSB0, /dev/cu.SLAB_USBtoUART). Optional.
#   MONITOR_BAUD Baud rate for serial monitor (default: 115200)

ACTION="${ACTION:-build}"
ENV_NAME="${ENV_NAME:-esp32vn-iot-uno}"
UPLOAD_PORT="${UPLOAD_PORT:-}"
MONITOR_BAUD="${MONITOR_BAUD:-115200}"

echo "[pio-docker] ACTION=${ACTION} ENV_NAME=${ENV_NAME} UPLOAD_PORT=${UPLOAD_PORT}"

case "${ACTION}" in
  build)
    exec pio run -e "${ENV_NAME}"
    ;;
  upload)
    if [[ -n "${UPLOAD_PORT}" ]]; then
      exec pio run -e "${ENV_NAME}" -t upload --upload-port "${UPLOAD_PORT}"
    else
      echo "[pio-docker] Hint: set UPLOAD_PORT to your serial device (e.g., /dev/ttyUSB0). Proceeding without explicit port..." >&2
      exec pio run -e "${ENV_NAME}" -t upload
    fi
    ;;
  monitor)
    # Optional simple serial monitor (requires device mapped into container)
    if [[ -z "${UPLOAD_PORT}" ]]; then
      echo "[pio-docker] Please set UPLOAD_PORT to your serial device (e.g., /dev/ttyUSB0) for monitor." >&2
      exit 1
    fi
    exec pio device monitor -b "${MONITOR_BAUD}" -p "${UPLOAD_PORT}"
    ;;
  *)
    echo "[pio-docker] Unknown ACTION: ${ACTION}. Use build | upload | monitor" >&2
    exit 2
    ;;
esac
