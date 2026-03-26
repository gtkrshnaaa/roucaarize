def fib(n):
    if n <= 1: return n
    return fib(n - 1) + fib(n - 2)

def runLoop():
    sum_val = 0
    i = 0
    while i < 300000:
        sum_val += i
        i += 1
    return sum_val

def runArray():
    arr = []
    i = 0
    while i < 20000:
        arr.append(i)
        i += 1
    return len(arr)

def runMap():
    m = {}
    i = 0
    while i < 10000:
        m["k" + str(i)] = i
        i += 1
    return i

print("Starting benchmarks...")
print("Fib(23): " + str(fib(23)))
print("Loop Sum: " + str(runLoop()))
print("Array Size: " + str(runArray()))
print("Map Entries: " + str(runMap()))
print("Done!")
