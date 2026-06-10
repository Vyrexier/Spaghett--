# Spaghett--

> *Code all night, still not right.*  
> *Think it's fine — runtime decline.*  
> *One more fix, now it's twenty-six.*  
> *Compile and pray. That's the way.*

---

**Spaghett--** is a dynamically-typed scripting language with syntax inspired by Lua — written for those who believe other languages compile too fast, run too clean, and crash too politely.

While the rest of the world ships optimized runtimes, Spaghett-- is still thinking about it.

---

## Features

- Lua-like syntax — familiar enough to hurt
- `local` scoping — variables that know their place
- First-class functions and closures
- Tables as the one true data structure
- Coroutines with `spawn`, `yield`, `wait`, and `wrap`
- `pcall` for errors you saw coming
- `os.clock()` — to measure how long you've suffered

---

## Syntax Overview

### Variables

```lua
local x = 42
local name = "Spaghett"
local flag = true
local nothing = null
```

### Arithmetic & String Concatenation

```lua
local a = 10
local b = 3
printl(a + b)   -- 13
printl(a % b)   -- 1
printl("Spaghett" .. " v" .. "1.0")  -- Spaghett v1.0
```

### Conditionals

```lua
if x > 5 then
    print("big")
elseif x == 5 then
    print("exact")
else
    print("small")
end
```

### Loops

**While:**
```lua
local i = 1
while i <= 3 do
    print(i)
    i = i + 1
end
```

**For** — `start, stop, step`:
```lua
for i = 1, 5, 1 do
    print(i)
end
```

### Functions

```lua
func add(a, b)
    return a + b
end

print(add(3, 4))  -- 7
```

### Closures

```lua
func makeCounter()
    local count = 0
    return func()
        count = count + 1
        return count
    end
end

local counter = makeCounter()
printl(counter())  -- 1
printl(counter())  -- 2
printl(counter())  -- 3
```

### Tables

```lua
local t = {x = 1, y = 2}
printl(t.x)   -- 1
t.z = 3
printl(t.z)   -- 3
```

### Error Handling

```lua
local ok, err = pcall(func()
    error("something went wrong")
end)
printl(ok)   -- false
printl(err)  -- something went wrong
```

---

## Coroutines

Spaghett-- supports cooperative concurrency through a coroutine system.

```lua
-- Create and resume
local co = coroutine.create(func()
    printl("step 1")
    coroutine.yield()
    printl("step 2")
end)

coroutine.resume(co)
coroutine.resume(co)

-- Check status
printl(coroutine.status(co))  -- "dead"

-- Wrap as iterator
local iter = coroutine.wrap(func()
    for i = 1, 3, 1 do
        coroutine.yield()
    end
end)

iter()
iter()
iter()

-- Spawn concurrent tasks
coroutine.spawn(func()
    printl("A start")
    coroutine.wait(0)
    printl("A end")
end)

coroutine.spawn(func()
    printl("B start")
    coroutine.wait(0)
    printl("B end")
end)
```

---

## Standard Library (Partial)

| Function | Description |
|---|---|
| `print(x)` | Print without newline |
| `printl(x)` | Print with newline |
| `error(msg)` | Raise a runtime error |
| `pcall(fn)` | Protected call — returns `ok, err` |
| `os.clock()` | Returns CPU time in seconds |
| `coroutine.create(fn)` | Create a coroutine |
| `coroutine.resume(co)` | Resume a coroutine |
| `coroutine.yield()` | Suspend current coroutine |
| `coroutine.status(co)` | `"suspended"`, `"running"`, or `"dead"` |
| `coroutine.wrap(fn)` | Wrap coroutine as a callable |
| `coroutine.spawn(fn)` | Spawn a concurrent task |
| `coroutine.wait(n)` | Yield for `n` ticks |

---

## Philosophy

> *Write with care — bugs everywhere.*  
> *Code with grace — crash in your face.*

Spaghett-- does not promise speed.  
It does not promise stability.  
It promises **character** — and the quiet satisfaction of a runtime error at 2 AM that you almost deserve.

---

## License

Do whatever you want. We're not going to stop you.  
We're still stuck on a segfault from last Thursday.
