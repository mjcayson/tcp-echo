#!/usr/bin/env bash
set -euo pipefail

#check if a server instance is already running
EXISTING_ID="$(docker compose ps -q server || true)"

CLEANUP()
{
  # Only stop if we started it
  if [[ -z "${EXISTING_ID}" ]]; then
    echo "Stopping server..."
    docker compose stop server >/dev/null 2>&1 || true
    docker compose rm -f server >/dev/null 2>&1 || true
  fi
}
trap CLEANUP EXIT

if [[ -z "${EXISTING_ID}" ]]; then
  echo "Starting server..."
  docker compose up -d server
else
  echo "Reusing existing server container: ${EXISTING_ID}"
fi

echo "Waiting for server to become healthy..."
until [ "$(docker inspect --format='{{.State.Health.Status}}' "$(docker compose ps -q server)")" = "healthy" ]; do
  sleep 1
done
echo "Server is healthy."

echo "Running integration tests..."
docker compose run --rm -e ECHO_HOST=server -e ECHO_PORT=5000 builder \
  bash -lc "cd /build && ctest -R integration --output-on-failure"

echo "Integration tests complete."
