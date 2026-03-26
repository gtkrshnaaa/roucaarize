# Roucaarize Language Reference

## Table of Contents
1. [Path Resolution Rules](#path-resolution-rules)
2. [Basic Syntax](#basic-syntax)
3. [Data Types](#data-types)
4. [Control Flow](#control-flow)
5. [Functions](#functions)
6. [Structs](#structs)
7. [Module System](#module-system)

---

## Path Resolution Rules

> [!IMPORTANT]
> **CRITICAL: All file paths in Roucaarize are resolved RELATIVE to the source file being executed, NOT relative to the current working directory.**

This behavior is identical to:
- `#include` in C/C++
- `<a href>` in HTML
- `import` in Python (when using relative imports)

### Path Resolution Behavior

```rou
// File: /project/src/main.rou
import "utils.rou" as utils           // Resolves to: /project/src/utils.rou
import "lib/helper.rou" as helper     // Resolves to: /project/src/lib/helper.rou
import "../config.rou" as config      // Resolves to: /project/config.rou
```

### Examples

#### Example 1: Same Directory
```
project/
├── main.rou
└── utils.rou
```

```rou
// File: project/main.rou
import "utils.rou" as utils
```
Resolves to: `project/utils.rou`

#### Example 2: Subdirectory
```
project/
├── main.rou
└── lib/
    └── helper.rou
```

```rou
// File: project/main.rou
import "lib/helper.rou" as helper
```
Resolves to: `project/lib/helper.rou`

#### Example 3: Parent Directory
```
project/
├── config.rou
└── src/
    └── main.rou
```

```rou
// File: project/src/main.rou
import "../config.rou" as config
```
Resolves to: `project/config.rou`

#### Example 4: Nested Imports
```
project/
├── main.rou
└── modules/
    ├── importer.rou
    └── utils/
        └── helper.rou
```

```rou
// File: project/modules/importer.rou
import "utils/helper.rou" as helper
```
Resolves to: `project/modules/utils/helper.rou`

### Working Directory Independence

> [!WARNING]
> Path resolution is **INDEPENDENT** of where you run the `roucaarize` command.

```bash
# All of these produce the SAME result:
cd /project && roucaarize src/main.rou
cd /project/src && roucaarize main.rou
cd / && roucaarize /project/src/main.rou
```

In all cases, `import "utils.rou"` inside `main.rou` resolves to `/project/src/utils.rou`.

### Stdlib Path Resolution

The same relative path resolution rules apply to standard library methods (e.g., `fs`). When you pass a path to a function, it is resolved relative to the script file calling it.

#### Downward Navigation
Accessing files in subdirectories from the current script.

```rou
import stdlib fs as f

// Resolves to 'data/reports/january.txt' relative to this file
content = f.read("data/reports/january.txt")
```

#### Complex Path Traversal
You can navigate outside the current directory using `../`.

```rou
import stdlib fs as f

// Load from a parent directory's sibling
content = f.read("../../data/external/dataset.txt")
```

---

## Basic Syntax

### Comments

```rou
// Single-line comment

// Multi-line comments are multiple single-line comments
// like this
```

### Variables

```rou
x = 42              // Integer
y = 3.14            // Float
name = "Roucaarize" // String
flag = true         // Boolean
nothing = nil       // Nil
```

### Operators

**Arithmetic:**
```rou
a + b    // Addition
a - b    // Subtraction
a * b    // Multiplication
a / b    // Division
a % b    // Modulo
```

**Comparison:**
```rou
a == b   // Equal
a != b   // Not equal
a < b    // Less than
a <= b   // Less than or equal
a > b    // Greater than
a >= b   // Greater than or equal
```

**Logical:**
```rou
a and b  // Logical AND
a or b   // Logical OR
not a    // Logical NOT
```

---

## Data Types

### Primitives

```rou
nil      // Null/undefined value
true     // Boolean true
false    // Boolean false
42       // Integer (int64)
3.14     // Float (double)
"text"   // String
```

### Arrays

```rou
arr = [1, 2, 3, 4, 5]
arr[0]              // Access: 1
arr[0] = 10         // Modify
push(arr, 6)        // Add element
pop(arr)            // Remove last
len(arr)            // Get length
```

### Maps

```rou
config = {
    "host": "localhost",
    "port": 8080
}
config["host"]         // Access: "localhost"
config["timeout"] = 30 // Modify/Add
```

### Structs

```rou
struct Point {
    x,
    y
}

p = Point(10, 20)
p.x = 30
```

---

## Control Flow

### If / Else If / Else

```rou
if (x > 10) {
    print("Large")
} else if (x > 5) {
    print("Medium")
} else {
    print("Small")
}
```

### For Loop

```rou
fruits = ["apple", "banana"]
for (fruit in fruits) {
    print(fruit)
}
```

### While Loop

```rou
counter = 0
while (counter < 3) {
    print(counter)
    counter = counter + 1
}
```

### Try-Catch-Finally

```rou
try {
    result = 10 / 0
} catch (err) {
    print("Caught error: " + toString(err))
} finally {
    print("Cleanup")
}
```

---

## Functions

### Basic Functions

```rou
function add(a, b) {
    return a + b
}

result = add(10, 20)
```

### Closures

```rou
function makeCounter() {
    count = 0
    
    function increment() {
        count = count + 1
        return count
    }
    
    return increment
}

counter = makeCounter()
print(counter())  // 1
print(counter())  // 2
```

---

## Structs

```rou
struct Vector3 {
    x,
    y,
    z
}

function magnitudeSquared(v) {
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z)
}

v = Vector3(3, 4, 0)
magSq = magnitudeSquared(v)  // 25
```

---

## Module System

### Importing Files

> [!IMPORTANT]
> Paths are **relative to the importing file**, not the working directory.

```rou
import "utils.rou" as utils
import "lib/helper.rou" as helper
import "../config.rou" as config
```

### Importing Standard Library

```rou
import stdlib sys as s
import stdlib fs as f
import stdlib time as t
import stdlib proc as p
import stdlib string as str
```

### Using Imported Modules

```rou
import "utils.rou" as utils

result = utils.square(5)
```

---

## No-Underscore Policy

> [!CAUTION]
> Underscores `_` are **FORBIDDEN** in all identifiers.

```rou
// REJECTED
my_variable = 10
function calculate_sum(a, b) { }

// ACCEPTED
myVariable = 10
function calculateSum(a, b) { }
```

This policy ensures:
- Visual consistency
- Clean, modern code aesthetic
- No mixing of naming conventions

---

## Built-in Functions

| Function | Description | Example |
|----------|-------------|---------|
| `print(msg)` | Output to stdout | `print("Hello!")` |
| `len(obj)` | Get length | `len([1,2,3])` → 3 |
| `typeof(val)` | Get type name | `typeof(42)` → "int" |
| `toString(val)` | Convert to string | `toString(123)` → "123" |
| `push(arr, val)` | Append to array | `push(items, 4)` |
| `pop(arr)` | Remove last | `pop(items)` |

---

## Standard Library (Orchestration Modules)

### sys Module

```rou
import stdlib sys as s

s.exec(cmd)     // Execute shell command
s.spawn(cmd)    // Spawn process natively
s.getEnv(key)   // Read environment variable
s.setEnv(k, v)  // Set environment variable
s.hostname()    // Get system hostname
s.uptime()      // Get system uptime in seconds
s.getUid()      // Get user ID
s.exit(code)    // Exit process with code
```

### fs Module

```rou
import stdlib fs as f

f.read(path)          // Read file contents
f.write(path, content)// Write content to file
f.append(path, data)  // Append content to file
f.exists(path)        // Check if file exists
f.remove(path)        // Delete file
```

### net Module

```rou
import stdlib net as n

n.ping(host)    // Ping IP/domain
n.fetch(url)    // Curl URL
n.getIp()       // Get local IP address
```

### proc Module

```rou
import stdlib proc as p

p.list()           // List running processes
p.isRunning(name)  // Check if process runs by name or PID
p.pkill(name)      // Kill process by name
p.cpuUsage()       // Get CPU usage string
p.memUsage()       // Get Memory usage string
```

### string Module

```rou
import stdlib string as str

str.split(s, delim)         // Split string into array
str.replace(s, old, new)    // Replace substring
str.trim(s)                 // Remove whitespace
str.toLowerCase(s)          // Convert to lowercase
str.toUpperCase(s)          // Convert to uppercase
str.contains(s, sub)        // Check if string contains substring
```

### time Module

```rou
import stdlib time as t

t.timestamp()  // Unix timestamp
t.format(fmt)  // Format time (e.g. "%Y-%m-%d")
t.millis()     // Current time in milliseconds
t.sleep(ms)    // Delay execution
t.year()       // Current year
t.month()      // Current month
```
