#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   scripts/build.sh         # normal build (cached, auto progress)
#   scripts/build.sh debug   # cached build, verbose logs
#   scripts/build.sh clean   # no-cache build, verbose logs

export DOCKER_BUILDKIT=1

MODE="${1:-}"
PROGRESS="--progress=auto"
NO_CACHE=""

case "$MODE" in
  clean)
    NO_CACHE="--no-cache"
    PROGRESS="--progress=plain"
    ;;
  debug)
    PROGRESS="--progress=plain"
    ;;
  "" )
    # defaults already set
    ;;
  * )
    echo "Unknown option: $MODE"
    echo "Usage: $0 [clean|debug]"
    exit 1
    ;;
esac

echo ">>> Building Docker images (BuildKit=${DOCKER_BUILDKIT}, no-cache=${NO_CACHE:+yes}, progress=${PROGRESS#--progress=})"
docker compose build $NO_CACHE $PROGRESS
