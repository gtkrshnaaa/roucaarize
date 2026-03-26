<?php
class Obj {
    public $val = 0;
}

function printHeader() {
    echo "  -------------------------------------------------------------\n";
    printf("  %-15s | %15s | Time\n", "Benchmark", "Performance");
    echo "  -------------------------------------------------------------\n";
}

function printResult($name, $ops, $sec) {
    if ($sec == 0) $sec = 0.0001;
    printf("  %-15s | %15.2f OPS/sec | %.4fs\n", $name, $ops, $sec);
}

function benchInt() {
    $limit = 1000000000;
    $start = microtime(true);
    $i = 0;
    while ($i < $limit) {
        $i++;
    }
    $sec = microtime(true) - $start;
    if ($sec == 0) $sec = 0.0001;
    $ops = $limit / $sec;
    printResult("Integer Add", $ops, $sec);
}

function benchDouble() {
    $limit = 100000000;
    $val = 0.0;
    $start = microtime(true);
    $i = 0;
    while ($i < $limit) {
        $val += 1.1;
        $i++;
    }
    $sec = microtime(true) - $start;
    if ($sec == 0) $sec = 0.0001;
    $ops = $limit / $sec;
    printResult("Double Arith", $ops, $sec);
}

function benchString() {
    $limit = 500000;
    $start = microtime(true);
    $s = "";
    $i = 0;
    while ($i < $limit) {
        $s .= "a";
        $i++;
    }
    $sec = microtime(true) - $start;
    if ($sec == 0) $sec = 0.0001;
    $ops = $limit / $sec;
    printResult("String Concat", $ops, $sec);
}

function benchArray() {
    $limit = 1000000;
    $start = microtime(true);
    $arr = [];
    $i = 0;
    while ($i < $limit) {
        $arr[] = $i;
        $i++;
    }
    $sec = microtime(true) - $start;
    if ($sec == 0) $sec = 0.0001;
    $ops = $limit / $sec;
    printResult("Array Push", $ops, $sec);
}

function benchStruct() {
    $limit = 50000000;
    $o = new Obj();
    $start = microtime(true);
    $i = 0;
    while ($i < $limit) {
        $o->val = $i;
        $x = $o->val;
        $i++;
    }
    $sec = microtime(true) - $start;
    if ($sec == 0) $sec = 0.0001;
    $ops = $limit / $sec;
    printResult("Struct Access", $ops, $sec);
}

echo ">>> PHP 8 Benchmark Suite <<<\n";
printHeader();
benchInt();
benchDouble();
benchString();
benchArray();
benchStruct();
echo "  -------------------------------------------------------------\n";
