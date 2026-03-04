# QuantClaw Production Image — Multi-stage build
#
# Stages:
#   cpp-builder  — compiles C++ binary
#   node-builder — compiles Sidecar (TypeScript → JS)
#   runtime      — minimal Ubuntu image with both artifacts
#
# Build:
#   docker build -f scripts/Dockerfile -t quantclaw:$(cat scripts/DOCKER_VERSION) .
# Run:
#   docker run -d --name quantclaw \
#     -p 18800:18800 -p 18801:18801 \
#     -v quantclaw_data:/home/quantclaw/.quantclaw \
#     quantclaw gateway run

ARG UBUNTU_VERSION=22.04

# ── Stage 1: C++ build ───────────────────────────────────────────────────────
FROM ubuntu:${UBUNTU_VERSION} AS cpp-builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ca-certificates \
    libssl-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libspdlog-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN mkdir -p build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF .. \
    && make -j$(nproc) \
    && strip build/quantclaw 2>/dev/null || true

# ── Stage 2: Sidecar (Node.js) build ────────────────────────────────────────
FROM ubuntu:${UBUNTU_VERSION} AS node-builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && apt-get install -y --no-install-recommends nodejs \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY sidecar/ ./sidecar/

RUN cd sidecar \
    && npm ci --omit=dev \
    && npm run build

# ── Stage 3: Runtime ─────────────────────────────────────────────────────────
FROM ubuntu:${UBUNTU_VERSION} AS runtime

ARG VERSION
ARG BUILD_DATE
ARG VCS_REF

LABEL org.opencontainers.image.title="QuantClaw" \
      org.opencontainers.image.description="High-performance AI agent runtime (C++17)" \
      org.opencontainers.image.version="${VERSION}" \
      org.opencontainers.image.created="${BUILD_DATE}" \
      org.opencontainers.image.revision="${VCS_REF}" \
      org.opencontainers.image.source="https://github.com/QuantClaw/QuantClaw" \
      org.opencontainers.image.licenses="MIT"

ENV DEBIAN_FRONTEND=noninteractive

# Runtime C++ deps + Node.js runtime
RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    libcurl4 \
    libspdlog1 \
    zlib1g \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/* \
    && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && apt-get install -y --no-install-recommends nodejs \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user (mirrors ixwebsocket's adduser pattern)
RUN groupadd -r quantclaw && useradd -r -g quantclaw -m -d /home/quantclaw quantclaw

# Install C++ binary
COPY --from=cpp-builder --chown=root:root /src/build/quantclaw /usr/local/bin/quantclaw
RUN chmod +x /usr/local/bin/quantclaw \
    && ldd /usr/local/bin/quantclaw

# Install Sidecar
COPY --from=node-builder --chown=quantclaw:quantclaw /src/sidecar/dist /home/quantclaw/sidecar/dist
COPY --from=node-builder --chown=quantclaw:quantclaw /src/sidecar/node_modules /home/quantclaw/sidecar/node_modules

# Runtime workspace layout (OpenClaw-compatible: agents/main/)
RUN mkdir -p /home/quantclaw/.quantclaw/agents/main/workspace \
             /home/quantclaw/.quantclaw/agents/main/sessions \
             /home/quantclaw/.quantclaw/logs \
    && chown -R quantclaw:quantclaw /home/quantclaw/.quantclaw

VOLUME ["/home/quantclaw/.quantclaw"]

EXPOSE 18800 18801

USER quantclaw
WORKDIR /home/quantclaw

HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD quantclaw health || exit 1

ENTRYPOINT ["quantclaw"]
CMD ["gateway", "run"]
