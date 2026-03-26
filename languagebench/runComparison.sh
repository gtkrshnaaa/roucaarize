#!/bin/bash

# Determine script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Build Roucaarize first
cd "$ROOT_DIR" && make -s
cd "$SCRIPT_DIR"

REPORT_FILE="battleReport.txt"

# Detect System Specs
CPU_INFO=$(grep "model name" /proc/cpuinfo | head -n 1 | cut -d ':' -f 2 | xargs)
MEM_TOTAL=$(grep MemTotal /proc/meminfo | awk '{print $2/1024 " MB"}')
OS_INFO=$(cat /etc/os-release | grep PRETTY_NAME | cut -d '"' -f 2)

# Resource Monitor Wrapper function
function runBench() {
    /usr/bin/time -f "\n[System Resources]\n  Peak RAM : %M KB\n  CPU Load : %P\n  Total Time: %e s" "$@" 2>&1
}

{
echo "================================================================================"
echo "                   ULTIMATE LANGUAGE BENCHMARK BATTLE"
echo "================================================================================"
echo "Execution Date: $(date)"
echo ""
echo "SYSTEM SPECIFICATIONS:"
echo "  OS:  $OS_INFO"
echo "  CPU: $CPU_INFO"
echo "  RAM: $MEM_TOTAL"
echo "================================================================================"

# Helper for clearer separation
function printSep() {
    echo ""
    echo "================================================================================"
}

printSep
echo "--- Roucaarize ---"
echo "Arch: AST-Walking Interpreter (C++20 Native)"
runBench ../bin/roucaarize benchRoucaarize.rou

# PHP (Standard)
printSep
echo "--- PHP 8 (Standard) ---"
echo "Arch: Bytecode (Stack-based) - Zend (OpCache Only)"
runBench php -d opcache.jit_buffer_size=0 benchPhp.php

# PHP (JIT)
printSep
echo "--- PHP 8 (JIT) ---"
echo "Arch: Zend Engine + JIT Enabled (Max Power)"
runBench php -d opcache.enable_cli=1 -d opcache.jit=1255 -d opcache.jit_buffer_size=128M benchPhp.php

# Lua (Standard)
printSep
echo "--- Lua 5.4 ---"
echo "Arch: Bytecode (Register-based) - Standard Interpreter"
if command -v lua &> /dev/null; then
    runBench lua benchLua.lua
else
    echo "Lua not found."
fi

# LuaJIT
printSep
echo "--- LuaJIT ---"
echo "Arch: JIT Compiled - Trace-based Compiler"
if command -v luajit &> /dev/null; then
    runBench luajit benchLua.lua
else
    echo "LuaJIT not found."
fi

# Python
printSep
echo "--- Python 3 ---"
echo "Arch: Bytecode (Stack-based) - CPython Interpreter"
runBench python3 benchPython.py

printSep
echo "Benchmark Complete."

} | tee "$REPORT_FILE"

echo ""
echo "Detailed report saved to: $REPORT_FILE"
