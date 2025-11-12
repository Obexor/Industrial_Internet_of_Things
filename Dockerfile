# Container to build this PlatformIO ESP32 project
# Usage:
#   docker build -t iiot-esp32 .
#   docker run --rm -v %cd%:/workspace -w /workspace iiot-esp32 pio run -e esp32vn-iot-uno

FROM python:3.11-slim

ARG DEBIAN_FRONTEND=noninteractive

# System dependencies for PlatformIO toolchains and optional USB access
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       git \
       ca-certificates \
       curl \
       build-essential \
       udev \
    && rm -rf /var/lib/apt/lists/*

# Install PlatformIO Core (CLI)
RUN pip install --no-cache-dir platformio

# Avoid interactive prompts and auto-updates inside CI/containers
ENV PLATFORMIO_SETTING_ENABLE_TELEMETRY=false \
    PLATFORMIO_CORE_DIR=/root/.platformio \
    PIO_UNIT_TESTING_VERBOSE=1

# Workdir inside the container; project will be mounted here at runtime
WORKDIR /workspace

# Pre-fetch platforms and libs to leverage Docker layer caching where possible
# Copy only platformio.ini first; this lets us cache toolchains and lib deps
COPY platformio.ini /tmp/platformio.ini
WORKDIR /tmp

# Install the platform for the declared environment and any declared libs
# (This step is best-effort; it speeds up subsequent builds.)
RUN pio pkg install -e esp32vn-iot-uno --project-dir /tmp || true \
 && pio platform install espressif32 || true

# Reset workdir; at runtime the host project should be mounted at /workspace
WORKDIR /workspace

# Default command builds the provided environment
CMD ["pio", "run", "-e", "esp32vn-iot-uno"]
