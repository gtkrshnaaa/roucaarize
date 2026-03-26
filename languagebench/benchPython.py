import time

class Obj:
    def __init__(self):
        self.val = 0

def printHeader():
    print("  -------------------------------------------------------------")
    print(f"  {'Benchmark':<15} | {'Performance':>15} | Time")
    print("  -------------------------------------------------------------")

def printResult(name, ops, sec):
    if sec == 0: sec = 0.0001
    ops_str = f"{ops:,.2f}"
    print(f"  {name:<15} | {ops_str:>15} OPS/sec | {sec:.4f}s")

def benchInt():
    limit = 10000
    start = time.time()
    i = 0
    while i < limit:
        i += 1
    end = time.time()
    sec = end - start
    ops = limit / sec if sec > 0 else limit / 0.0001
    printResult("Integer Add", ops, sec)

def benchDouble():
    limit = 100000
    val = 0.0
    start = time.time()
    i = 0
    while i < limit:
        val += 1.1
        i += 1
    end = time.time()
    sec = end - start
    ops = limit / sec if sec > 0 else limit / 0.0001
    printResult("Double Arith", ops, sec)

def benchString():
    limit = 5000
    start = time.time()
    s = ""
    i = 0
    while i < limit:
        s += "a"
        i += 1
    end = time.time()
    sec = end - start
    ops = limit / sec if sec > 0 else limit / 0.0001
    printResult("String Concat", ops, sec)

def benchArray():
    limit = 10000
    start = time.time()
    arr = []
    i = 0
    while i < limit:
        arr.append(i)
        i += 1
    end = time.time()
    sec = end - start
    ops = limit / sec if sec > 0 else limit / 0.0001
    printResult("Array Push", ops, sec)

def benchStruct():
    limit = 500000
    o = Obj()
    start = time.time()
    i = 0
    while i < limit:
        o.val = i
        x = o.val
        i += 1
    end = time.time()
    sec = end - start
    ops = limit / sec if sec > 0 else limit / 0.0001
    printResult("Struct Access", ops, sec)

if __name__ == "__main__":
    print(">>> Python 3 Benchmark Suite <<<")
    printHeader()
    benchInt()
    benchDouble()
    benchString()
    benchArray()
    benchStruct()
    print("  -------------------------------------------------------------")
    print("")
