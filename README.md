<p align="center">
  <img src="docs/assets/img/routextlogo.svg" alt="ROUCAARIZE" width="900">
</p>

---

**Roucaarize** is a minimalist, "cute and clean" programming language designed specifically for **Linux orchestration**, system administration, and daemon management. It serves as a safer, strongly-typed alternative to Bash, prioritizing maintainability and low memory footprint.

> **Philosophy:** Roucaarize is built to be simple. Instead of a complex JIT, it uses a highly efficient **tree-walking interpreter** that prioritizes RAM efficiency and clean code over raw execution speed. It's the "cute" way to manage your Linux infrastructure.

---

## Why Roucaarize?

- **Low Memory Footprint:** Designed to run on minimal resources, perfect for small containers and background daemons.
- **Strongly Typed Orchestration:** No more shell script exit code nightmares — use `try-catch` for system operations.
- **Grammar-First Development:** Integrated static analyzer ensures your scripts follow best practices before they even run.
- **Maintainable Syntax:** Enforces the **No-Underscore Policy** (`camelCase` only) to keep your orchestration scripts readable.

---

## A Taste of Roucaarize

Managing services and files has never been this clean:

```rou
import stdlib sys as s
import stdlib fs as f

// Simple function to ensure a service is running
function ensureService(name) {
    if (!s.systemctl("is-active", name)) {
        print("Starting service:", name)
        s.systemctl("start", name)
    }
}

// Orchestrate your setup
services = ["nginx", "redis", "postgresql"]
for (svc in services) {
    ensureService(svc)
}

print("System is healthy!")
```

---

## Static Analysis (Grammar Check)

Roucaarize encourages a "correct by construction" approach. Use the built-in grammar checker to validate your scripts:

```bash
# Check syntax, naming, and orchestration best practices
./bin/roucaarize -grammar setup.rou
# or
./bin/roucaarize -g setup.rou
```

---

## Architecture

Roucaarize follows a simplified version of the Nevaarize pipeline:

```
Source Code (.rou)
       ↓
   [Lexer]     → Token stream
       ↓
   [Parser]    → Abstract Syntax Tree
       ↓
  [Interpreter] → Tree-walking execution
```

**Key internals:**
- **Tree-walking evaluator** — Simple, reliable, and memory-efficient.
- **Value system** — Tagged unions with shared reference semantics.
- **Zero-copy lexing** — High-performance tokenization via `std::string_view`.
- **Static analyzer** — Deep grammar validation baked into the core.

---

## Standard Library (Orchestration-Ready)

| Module | Import | Functions |
|--------|--------|-----------|
| **Sys** | `import stdlib sys as s` | `s.exec()`, `s.spawn()`, `s.systemctl()`, `s.getEnv()` |
| **Fs** | `import stdlib fs as f` | `f.read()`, `f.write()`, `f.chmod()`, `f.mount()` |
| **Net** | `import stdlib net as n` | `n.ping()`, `n.fetch()`, `n.listen()`, `n.connect()` |
| **Proc** | `import stdlib proc as p` | `p.list()`, `p.isRunning()`, `p.pkill()`, `p.usage()` |

---

## License

MIT License — Built for the Linux community.
