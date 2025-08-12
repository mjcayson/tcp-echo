#!/usr/bin/env bash

set -euo pipefail
docker compose run --rm builder bash -lc "cd /build && ctest -R integration --output-on-failure"
