# Hermes-islands

A minimal, grand design prototype: classloader “islands” with an event bus (Hermes) to observe loading, invocation, allocation, GC, and controlled unload. Built to teach visibility, isolation, and the economy of memory without drowning in code. Beyond the metaphor, this architecture solves real, concrete problems in long‑running systems.

---

## 1. Plugin Systems with Safe Unload

**Problem:**  
Large applications (IDEs, servers, financial platforms) accumulate plugins in memory. Traditional JVM classloaders don’t easily release them, so heap grows until restart.

**Solution with Hermes‑islands:**  
- Each plugin runs in its own island (classloader).  
- When disabled, the island is unloaded → all its classes and objects become collectible.  
- Heap stays small, uptime increases.

**Example:** Eclipse or IntelliJ plugins, or a trading platform with dynamic strategy modules.

---

## 2. Microservices Inside One JVM

**Problem:**  
Running multiple services in one JVM risks classpath conflicts and memory bloat.

**Solution:**  
- Each service runs in its own island with isolated class tables.  
- Hermes events monitor load/invoke, tracking resource usage per service.  
- If one service exceeds its budget, Hermes can trigger `UNLOAD_REQUEST`.

**Example:** A financial simulation engine where each model is a separate island.

---

## 3. Hot Reload Without Reflection

**Problem:**  
Developers want to reload modules at runtime without reflection hacks or DI frameworks.

**Solution:**  
- Hermes‑islands allow you to drop an old island and load a new one with updated classes.  
- Events make the reload visible and auditable.

**Example:** Web server modules, or algorithmic trading strategies swapped live.

---

## 4. Economy of Heap (Accounting)

**Problem:**  
In long‑running systems, memory leaks are invisible until too late.

**Solution:**  
- Hermes events (`ALLOC`, `GC_MINOR`, `GC_MAJOR`) provide per‑island accounting.  
- You can see which module consumes memory, and enforce budgets.

**Example:** A simulation platform where each scenario is an island with a strict memory cap.

---

## 5. Security and Policy Enforcement

**Problem:**  
Preventing certain classes or APIs from being loaded.

**Solution:**  
- Hermes intercepts `LOAD_REQUEST` and applies whitelist/blacklist rules.  
- Suspicious modules can be refused or sandboxed.

**Example:** Preventing plugins from using `java.io.File` or `sun.misc.Unsafe`.

---

## Metaphor

Hermes‑islands are like **market stalls in an agora**:  
- Each merchant (module) has its own stall (classloader).  
- Hermes walks the market, taxing, auditing, and closing stalls when they break the rules.  
- The agora (VM) stays clean, ordered, and efficient.

---

## Concrete Win

**Long‑running systems stay lean and safe** because you can load, run, and unload modules dynamically, with full visibility and accounting.

---

## Overview

- **Goal:** Keep the heap small and the system efficient by isolating modules into classloader islands and making their life cycle visible via events.
- **Core idea:** **Aspect on the gates, not on the knights.** Monitor loading/unloading and method invocation at the boundaries.
- **Why:** **Memory is currency.** Isolation lets you release entire islands and reclaim their objects cleanly.

<img src="A grand visual poste.png" width="50%"/>

---

## Features

- **Islands:** Separate classloader per module with its own class table and memory budget.
- **Hermes events:** **LOAD**, **VERIFY/LINK**, **INVOKE**, **ALLOC**, **GC**, **UNLOAD** for full visibility.
- **Economy rules:** **Budgets per island**, **tax per invoke**, **black/white lists** for classes/resources.
- **Controlled unload:** Close gates, drop references, let GC reclaim the island’s objects.

---

## Quick start

- **Clone:** Put this README at repo root. Create `src/` for your prototype stubs.
- **Run flow:** One island loads a module, runs `main`, then unloads on request.
- **Demo target:** Use a JAR with `Main-Class` in its manifest for simple discovery.

#### Suggested layout

- **Root:**
  - `CMakeLists.txt` — minimal build
  - `src/` — core stubs
  - `demo/` — sample JARs, scripts
  - `docs/` — diagrams and slides

#### Minimal CMake

```cmake
cmake_minimum_required(VERSION 3.16)
project(hermes_islands CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(hermes_islands
  src/main.cpp
  src/event_bus.hpp
  src/event_bus.cpp
  src/island.hpp
  src/island.cpp
  src/vm.hpp
  src/vm.cpp
)
```

---

## Event contract

- **LOAD_REQUEST:** Ask to load a class/module.
- **LOAD_OK / LOAD_FAIL:** Confirm or reject load.
- **VERIFY_OK / LINK_OK:** Type safety and symbol resolution succeeded.
- **INVOKE_START / INVOKE_END:** A method (e.g., `main`) begins/ends execution.
- **ALLOC:** An object allocation counted against the island’s budget.
- **GC_MINOR / GC_MAJOR:** Garbage collections reported with pressure metrics.
- **UNLOAD_REQUEST / UNLOAD_OK / UNLOAD_FAIL:** Module unload cycle.

- **Rules (examples):**
  - **Budget per island:** Over budget triggers **THROTTLE** or **UNLOAD_REQUEST**.
  - **Tax per invoke:** Aggregate cost per method for accounting.
  - **Class/resource filters:** Blacklist/whitelist at **LOAD_REQUEST**.

---

## Roadmap

- **Step 1:** Implement EventBus and stub methods that only emit events.
- **Step 2:** Add simple JAR discovery and manifest parsing (for `Main-Class`).
- **Step 3:** Simulate budgets and memory pressure to exercise unload logic.
- **Step 4:** Introduce real class loading and a basic interpreter later, if desired.

# Worked example: Plugin system for a financial simulation engine

A concrete scenario showing how classloader islands and Hermes events track memory usage per plugin and enable safe unload when budgets are exceeded or runs finish.

<img src="Diagramă conceptuală.png" width="30%"/>

---

## System context

- **Core engine:** Runs simulations (Monte Carlo, scenario backtesting) and orchestrates plugins.
- **Plugins:** Strategy modules (e.g., MeanReversion, Momentum, RiskParity) packaged as JARs with a `Main` entrypoint and a set of services.
- **Islands:** Each plugin has its own classloader island, memory budget, and lifecycle.
- **Hermes (EventBus):** Emits and listens to events for load, invoke, allocation, GC, and unload; keeps per‑island accounting.

---

## Roles and components

- **Engine:** Coordinates simulation runs, creates islands, enforces budgets.
- **Island:** Loads plugin JAR, links classes, runs `main`, records allocations, supports unload.
- **Hermes events:** Provide visibility and rules.
- **Accounting service:** Aggregates memory and invocation costs per island; triggers throttling/unload.

---

## Plugin lifecycle flow

1. **Discover:** Engine finds `plugins/*.jar` and reads manifest for `Main-Class`.
2. **Create island:** Engine assigns a budget (e.g., 128 MB) and registers event handlers.
3. **Load & link:** Island loads the plugin JAR, verifies and links classes.
4. **Run:** Engine invokes plugin `Main` with simulation parameters.
5. **Track:** Hermes records `ALLOC`, `INVOKE_*`, `GC_*` events, updates plugin’s account.
6. **Decide:** If budget exceeded or run completed, engine triggers `UNLOAD_REQUEST`.
7. **Unload:** Island closes, drops references; GC reclaims memory; Hermes records `UNLOAD_OK`.

---

## Event contract used

- **LOAD_REQUEST / LOAD_OK / LOAD_FAIL**
- **VERIFY_OK / LINK_OK**
- **INVOKE_START / INVOKE_END**
- **ALLOC (bytes)**
- **GC_MINOR / GC_MAJOR (bytes freed)**
- **UNLOAD_REQUEST / UNLOAD_OK / UNLOAD_FAIL**
- Optional: **THROTTLE** when nearing budget, **BUDGET_EXCEEDED** when over.

---

## Example scenario run

### Setup

- **Plugins:** `MeanReversion.jar`, `Momentum.jar`
- **Budgets:** `MeanReversion = 128 MB`, `Momentum = 96 MB`
- **Simulation:** 1,000 scenarios, 10 days, tick data subset

### Event timeline (abridged logs)

```text
[Hermes] LOAD_REQUEST: plugins/MeanReversion.jar
[Hermes] LOAD_OK: plugins/MeanReversion.jar
[Hermes] VERIFY_OK: island=MeanReversion
[Hermes] LINK_OK: island=MeanReversion

[Hermes] INVOKE_START: com.example.meanrev.Main
[Hermes] ALLOC: island=MeanReversion bytes=4,194,304  // warm-up data
[Hermes] ALLOC: island=MeanReversion bytes=12,582,912 // model state
[Hermes] GC_MINOR: island=MeanReversion freed=2,097,152
[Hermes] ALLOC: island=MeanReversion bytes=8,388,608  // scenario buffer
[Hermes] THROTTLE: island=MeanReversion usage=110MB/128MB

[Hermes] INVOKE_START: com.example.meanrev.RunScenario(1..250)
[Hermes] ALLOC: island=MeanReversion bytes=16,777,216
[Hermes] BUDGET_EXCEEDED: island=MeanReversion usage=132MB/128MB
[Hermes] UNLOAD_REQUEST: island=MeanReversion reason=BudgetExceeded
[Hermes] INVOKE_END: com.example.meanrev.RunScenario(1..250)
[Hermes] UNLOAD_OK: island=MeanReversion reclaimed≈120MB
```

```text
[Hermes] LOAD_REQUEST: plugins/Momentum.jar
[Hermes] LOAD_OK: plugins/Momentum.jar
[Hermes] VERIFY_OK: island=Momentum
[Hermes] LINK_OK: island=Momentum

[Hermes] INVOKE_START: com.example.momentum.Main
[Hermes] ALLOC: island=Momentum bytes=6,291,456
[Hermes] GC_MINOR: island=Momentum freed=1,048,576
[Hermes] INVOKE_END: com.example.momentum.Main
[Hermes] UNLOAD_REQUEST: island=Momentum reason=RunCompleted
[Hermes] UNLOAD_OK: island=Momentum reclaimed≈70MB
```

### Interpretation

- **MeanReversion** breached its budget; Hermes triggered a controlled unload, preventing heap bloat.
- **Momentum** completed its run; engine unloaded the island, reclaiming memory immediately.

---

## Accounting rules (economy layer)

- **Budget per island:** Track cumulative `ALLOC − GC` for live footprint; if `> budget`, raise `BUDGET_EXCEEDED`.
- **Soft throttle:** At `>85%` budget, emit `THROTTLE` to slow scenario batches (sleep, smaller buffers).
- **Hard unload:** On `BUDGET_EXCEEDED`, emit `UNLOAD_REQUEST`; engine stops new invocations and drains the island.
- **Cost per invocation:** Record time and bytes per `INVOKE_*` to identify expensive phases.
- **Policy filters:** Reject loading of forbidden classes/APIs at `LOAD_REQUEST`.

---

## Minimal pseudocode for the engine

```cpp
// Create islands and bind accounting
Island& meanRev = vm.createIsland("MeanReversion", 128_MB);
Island& momentum = vm.createIsland("Momentum", 96_MB);

// Register Hermes handlers
auto& bus = vm.events();
size_t usageMR = 0, usageMM = 0;

bus.on(EventType::ALLOC, [&](const Event& e){
  if (e.subject == "MeanReversion") usageMR += e.value;
  if (e.subject == "Momentum") usageMM += e.value;
  if (usageMR > 0.85 * 128_MB) bus.emit({EventType::UNLOAD_REQUEST, "MeanReversion"});
  if (usageMM > 0.85 * 96_MB)  bus.emit({EventType::UNLOAD_REQUEST, "Momentum"});
});

bus.on(EventType::UNLOAD_REQUEST, [&](const Event& e){
  // stop scheduling new scenarios for that island
  if (e.subject == "MeanReversion") meanRev.unload();
  if (e.subject == "Momentum")      momentum.unload();
});

// Run lifecycle
meanRev.loadModule("plugins/MeanReversion.jar");
meanRev.linkAll();
vm.runMain(meanRev, "com.example.meanrev.Main");

momentum.loadModule("plugins/Momentum.jar");
momentum.linkAll();
vm.runMain(momentum, "com.example.momentum.Main");
```

Note: This pseudocode shows the control flow and event responses; the actual memory values would be fed by instrumentation inside each island.

---

## Safe unload mechanics

- **Isolation:** Each plugin lives in its island with its own class table; no shared static references across islands.
- **Draining:** On `UNLOAD_REQUEST`, engine stops new invocations, waits for in‑flight calls to finish, closes resources.
- **Reference drop:** Island clears caches and registries; only weak references remain.
- **GC reclaim:** With no strong references, GC can reclaim objects from the island; Hermes logs reclaimed estimates via `GC_*`.
- **Verification:** `UNLOAD_OK` confirms cleanup completion; engine can now load a newer plugin version if needed.

---

## Operational benefits

- **Predictable memory:** Per‑plugin budgets make heap usage visible and enforceable.
- **Uptime:** You avoid “restart to recover memory”; islands let you reclaim on demand.
- **Auditable runs:** Event logs provide a trace of load, invoke, allocation, GC, and unload decisions.
- **Controlled hot‑swap:** Replace plugins mid‑session by unloading old islands and loading new ones.

---

## Teaching/demo script

- **Step 1:** Start engine; show initial Hermes dashboard (0 islands).
- **Step 2:** Load MeanReversion; watch `LOAD_*`, `VERIFY/LINK`, and first `ALLOC`.
- **Step 3:** Run scenarios; observe `INVOKE_*`, memory growth, `THROTTLE`.
- **Step 4:** Trigger budget breach; show `UNLOAD_REQUEST` → `UNLOAD_OK`; heap drops.
- **Step 5:** Load Momentum; run and cleanly unload on completion.
- **Step 6:** Discuss how isolation enabled immediate memory recovery and safe plugin turnover.

---

## Extensions

- **Per‑phase budgets:** Different caps for warm‑up, training, inference.
- **Rate limits:** Limit `INVOKE_START` frequency per island to stabilize CPU.
- **Policy packs:** Security filters for I/O, network, and native calls at `LOAD_REQUEST`.
- **Snapshots:** Emit periodic `USAGE_SNAPSHOT` events for real‑time dashboards.

---

This worked example demonstrates, end‑to‑end, how Hermes‑islands keep a financial simulation engine lean and reliable: isolate plugins, track their memory as a first‑class metric, and safely unload when necessary.

# Plugin Demo: MeanReversion Strategy

A minimal plugin for a financial simulation engine using Hermes-islands architecture. This plugin runs in its own classloader island, respects memory budgets, and can be safely unloaded after execution.

---

## 📁 Structure

```
MeanReversion.jar
├── META-INF/
│   └── MANIFEST.MF
├── com/
│   └── example/
│       └── meanrev/
│           ├── Main.java
│           ├── Strategy.java
│           └── Config.properties
```

---

## 🧾 Manifest (`META-INF/MANIFEST.MF`)

```properties
Manifest-Version: 1.0
Main-Class: com.example.meanrev.Main
Plugin-Name: MeanReversion
Plugin-Version: 1.2.0
```

---

## 🧩 Plugin Interface

```java
package engine;

public interface PluginEntry {
    void run(SimulationContext ctx);
}
```

---

## 🧠 Main Class

```java
package com.example.meanrev;

import engine.PluginEntry;
import engine.SimulationContext;

public class Main implements PluginEntry {
    @Override
    public void run(SimulationContext ctx) {
        Strategy strategy = new Strategy();
        strategy.train(ctx.getHistoricalData());
        ctx.submitResults(strategy.evaluate(ctx.getScenario()));
    }
}
```

---

## 📊 Strategy Class

```java
package com.example.meanrev;

import java.util.List;

public class Strategy {
    public void train(List<Double> historicalData) {
        // Simulate training logic
    }

    public double evaluate(List<Double> scenarioData) {
        // Simulate evaluation logic
        return Math.random(); // placeholder
    }
}
```

---

## ⚙️ Config File (`Config.properties`)

```properties
lookback=20
threshold=0.05
riskLevel=medium
```

---

## 🔄 Lifecycle in Hermes-Islands

1. **LOAD_REQUEST:** Engine discovers JAR and reads manifest.
2. **LOAD_OK:** Island loads classes.
3. **VERIFY_OK / LINK_OK:** Bytecode verified and symbols resolved.
4. **INVOKE_START:** Engine calls `Main.run(ctx)`.
5. **ALLOC:** Strategy allocates memory for training and evaluation.
6. **INVOKE_END:** Execution completes.
7. **UNLOAD_REQUEST / UNLOAD_OK:** Plugin is safely unloaded; memory reclaimed.

---

## 🧮 Budget Behavior

- Plugin is assigned 128 MB heap budget.
- Hermes tracks `ALLOC` events.
- If usage exceeds budget, engine triggers `UNLOAD_REQUEST`.
- Plugin drops references; GC reclaims memory.

---

## 🛡️ Best Practices

- No static references to engine or other plugins.
- No direct I/O or network calls without permission.
- All logic contained within the plugin island.
- Configurable via external `.properties` file.

---

## 🧪 Testing

- Place JAR in `plugins/` folder.
- Run engine with Hermes-islands enabled.
- Observe Hermes event logs for `LOAD`, `INVOKE`, `ALLOC`, `UNLOAD`.

---
When the plugin finishes its trade, Hermes closes the gate and the air returns to the agora.

```
