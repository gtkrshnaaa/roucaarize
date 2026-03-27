<p align="center">
  <img src="docs/assets/img/routextlogo.svg" alt="roucaarize" width="600">
</p>

---

### **roucaarize**
*just a tiny, cute language for your linux stuff... it's not that deep, fr.*

---

**roucaarize** is like... a baby project. it's a very smol and quiet programming language for **linux orchestration**. it doesn't try to be the fastest or the biggest. it just wants to be cute, clean, and not eat all your ram. no cap.

> **vibe check:** we don't have a big scary jit or anything. we're just a simple **tree-walking interpreter**. it's kinda slow (fr), but it's very gentle on your memory. think of it as a cozy alternative to bash scripts when you're tired of the noise.

---

### why roucaarize? (she's just a girl...)

- **tiny ram vibes:** she only needs like ~5mb to exist. perfect for your tiny containers. 
- **soft orchestration:** no more scary shell errors. it's all dynamic and chill, we use `try-catch` because we like being safe and comfy.
- **cute syntax:** we only use `camelCase`. no underscores allowed, they're too loud. 
- **honest speed:** we're not winning any races. we're just walking through the tree, one step at a time. it's peaceful.

---

### a smol taste 

managing services is kinda cute when you do it like this:

```rou
import stdlib sys as sys
import stdlib time as t

// just a smol helper to say hi to our services
function waveAt(pal) {
    if (sys.exec("systemctl is-active " + pal) != "active") {
        sys.spawn("systemctl start " + pal)
    }
}

// waving at our pals
pals = ["nginx", "redis"]
for (pal in pals) {
    waveAt(pal)
    t.sleep(100) // take a breath
}
```

---

### architecture (the inner peace)

we don't do anything too fancy. it's just a quiet pipeline:
`source` -> `lexer` -> `parser` -> `the walker`

- **tree-walker:** we just walk the tree. it's slow, but it's honest work.
- **micro-stack:** our async stuff uses tiny 512kb stacks. we're very shy with your memory.
- **iterative analysis:** the grammar checker won't crash your system. it's built to stay calm even with deep code.

---

### standard library (smol but helpful)

| module | import | what she does |
|--------|--------|-----------|
| **sys** | `as sys` | talks to linux, gently |
| **fs** | `as fs` | touches files with care |
| **t** | `as t` | knows what time it is |
| **arr** | `as arr` | holds your things together |

---

## license

mit. just take it. it's for the community, fr. 🧸
