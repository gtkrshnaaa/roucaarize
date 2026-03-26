local obj = {}
obj.__index = obj

function obj:new()
    local o = {val = 0}
    setmetatable(o, obj)
    return o
end

local function printHeader()
    print("  -------------------------------------------------------------")
    print(string.format("  %-15s | %15s | Time", "Benchmark", "Performance"))
    print("  -------------------------------------------------------------")
end

local function printResult(name, ops, sec)
    if sec == 0 then sec = 0.0001 end
    print(string.format("  %-15s | %15.2f OPS/sec | %.4fs", name, ops, sec))
end

local function benchInt()
    local limit = 10000
    local start = os.clock()
    local i = 0
    while i < limit do
        i = i + 1
    end
    local sec = os.clock() - start
    if sec == 0 then sec = 0.0001 end
    local ops = limit / sec
    printResult("Integer Add", ops, sec)
end

local function benchDouble()
    local limit = 100000
    local val = 0.0
    local start = os.clock()
    local i = 0
    while i < limit do
        val = val + 1.1
        i = i + 1
    end
    local sec = os.clock() - start
    if sec == 0 then sec = 0.0001 end
    local ops = limit / sec
    printResult("Double Arith", ops, sec)
end

local function benchString()
    local limit = 5000
    local start = os.clock()
    local s = ""
    local i = 0
    while i < limit do
        s = s .. "a"
        i = i + 1
    end
    local sec = os.clock() - start
    if sec == 0 then sec = 0.0001 end
    local ops = limit / sec
    printResult("String Concat", ops, sec)
end

local function benchArray()
    local limit = 10000
    local start = os.clock()
    local arr = {}
    local i = 0
    while i < limit do
        table.insert(arr, i)
        i = i + 1
    end
    local sec = os.clock() - start
    if sec == 0 then sec = 0.0001 end
    local ops = limit / sec
    printResult("Array Push", ops, sec)
end

local function benchStruct()
    local limit = 500000
    local o = obj:new()
    local start = os.clock()
    local i = 0
    while i < limit do
        o.val = i
        local x = o.val
        i = i + 1
    end
    local sec = os.clock() - start
    if sec == 0 then sec = 0.0001 end
    local ops = limit / sec
    printResult("Struct Access", ops, sec)
end

print(">>> Lua Benchmark Suite <<<")
printHeader()
benchInt()
benchDouble()
benchString()
benchArray()
benchStruct()
print("  -------------------------------------------------------------")
