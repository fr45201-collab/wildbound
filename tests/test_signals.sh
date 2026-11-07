#!/bin/bash
set -e
cd /mnt/c/dev/Wildbound/build/linux/gcc-release

echo "=== Testing SIGINT Handling ==="
./WildboundServer --port 7781 &
PID1=$!
sleep 1.5
echo "Sending SIGINT (kill -2) to PID $PID1..."
kill -2 $PID1
wait $PID1 || true
echo "SIGINT test complete."

echo "=== Testing SIGTERM Handling ==="
./WildboundServer --port 7782 &
PID2=$!
sleep 1.5
echo "Sending SIGTERM (kill -15) to PID $PID2..."
kill -15 $PID2
wait $PID2 || true
echo "SIGTERM test complete."

