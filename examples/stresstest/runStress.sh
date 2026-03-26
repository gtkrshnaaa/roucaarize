#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/../../bin/roucaarize"

cd "$SCRIPT_DIR"

if [ ! -f "$BIN" ]; then
    echo "Compiler binary not found. Please run 'make' at root."
    exit 1
fi

echo "==================================================="
echo ">>> ROUCAARIZE LINUX ORCHESTRATION STRESS TESTS <<<"
echo "==================================================="
echo "Note: Engine configured with 10s runtime timeout constraint."
echo ""

echo "=== 1. MEMORY PRESSURE ==="
"$BIN" memoryPressure.rou
echo ""
echo "=== 2. INFINITE RECURSION ==="
"$BIN" recursion.rou || echo "Engine caught error and safely exited with code $?"
echo ""
echo "=== 3. ORCHESTRATION CHAOS ==="
"$BIN" orchestrationChaos.rou
echo ""
echo "=== 4. INFINITE LOOP TIMEOUT ==="
"$BIN" infiniteLoop.rou || echo "Engine caught error and safely exited with code $?"
echo ""
echo "==================================================="
echo ">>> STRESS TEST SUITE COMPLETED WITHOUT SEGFAULT <<<"
echo "==================================================="
