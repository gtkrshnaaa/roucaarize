<?php
function fib($n) {
    if ($n <= 1) return $n;
    return fib($n - 1) + fib($n - 2);
}

function runLoop() {
    $sum = 0;
    $i = 0;
    while ($i < 300000) {
        $sum = $sum + $i;
        $i = $i + 1;
    }
    return $sum;
}

function runArray() {
    $arr = [];
    $i = 0;
    while ($i < 20000) {
        $arr[] = $i;
        $i = $i + 1;
    }
    return count($arr);
}

function runMap() {
    $m = [];
    $i = 0;
    while ($i < 10000) {
        $m["k" . $i] = $i;
        $i = $i + 1;
    }
    return $i;
}

echo "Starting benchmarks...\n";
echo "Fib(23): " . fib(23) . "\n";
echo "Loop Sum: " . runLoop() . "\n";
echo "Array Size: " . runArray() . "\n";
echo "Map Entries: " . runMap() . "\n";
echo "Done!\n";
