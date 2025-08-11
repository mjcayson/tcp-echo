#!/usr/bin/env bash

set -euo pipefail
#run tests inside builder image
#uses the already-built /build tree
docker compose run --rm builder bash -lc "cd /build && ctest -R unit --output-on-failure"
