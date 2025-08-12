#!/usr/bin/env bash

set -euo pipefail
# Usage: spawn_clients.sh <count> [host] [port] [user] [pass] [msg]
COUNT="${1:-1}"
HOST="${2:-server}"
PORT="${3:-5000}"
USER="${4:-testuser}"
PASS="${5:-testpass}"
MSG="${6:-hello from client}"

echo "Waiting for docker's healthy status..."
until [ "$(docker inspect --format='{{.State.Health.Status}}' $(docker compose ps -q server))" = "healthy" ]; do
  sleep 1
done

pids=()
for i in $(seq 1 "$COUNT"); do
  docker compose run --rm -T runtime \
    /app/client --host "$HOST" --port "$PORT" \
    --user "$USER" --pass "$PASS" --msg "$MSG" &
  pids+=($!)
done

for pid in "${pids[@]}"; do
  wait "$pid"
done
