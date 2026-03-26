local function fib(n)
    if n <= 1 then return n end
    return fib(n - 1) + fib(n - 2)
end

local function runLoop()
    local sum = 0
    local i = 0
    while i < 300000 do
        sum = sum + i
        i = i + 1
    end
    return sum
end

local function runArray()
    local arr = {}
    local i = 0
    while i < 20000 do
        table.insert(arr, i)
        i = i + 1
    end
    return #arr
end

local function runMap()
    local m = {}
    local i = 0
    while i < 10000 do
        m["k" .. tostring(i)] = i
        i = i + 1
    end
    return i
end

print("Starting benchmarks...")
print("Fib(23): " .. tostring(fib(23)))
print("Loop Sum: " .. tostring(runLoop()))
print("Array Size: " .. tostring(runArray()))
print("Map Entries: " .. tostring(runMap()))
print("Done!")
