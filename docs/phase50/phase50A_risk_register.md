# WILDBOUND — PHASE 50A: RISK REGISTER
**Document Reference:** `docs/phase50/phase50A_risk_register.md`  
**Parent Document:** Phase 50A Initial Repository Audit  
**Execution Mode:** STRICT AUDIT-ONLY  
**Date:** 2026-11-08  

---

## 1. Risk Evaluation Matrix

Risks are evaluated on a $5 \times 5$ grid of **Likelihood** ($1 = \text{Rare}$ to $5 = \text{Almost Certain}$) and **Impact** ($1 = \text{Negligible}$ to $5 = \text{Catastrophic}$).

```text
IMPACT
  5 |           RSK-001   RSK-002
  4 | RSK-005   RSK-003
  3 |           RSK-004
  2 | RSK-006
  1 |
    +-----------------------------
      1    2    3    4    5  LIKELIHOOD
```

---

## 2. Identified Production & Architectural Risks

### RSK-001: Headless Server GPU Dependency Drift
- **Category:** Production Deployment
- **Likelihood:** 3 (Moderate) | **Impact:** 5 (Catastrophic) | **Risk Score:** **15 (HIGH)**
- **Description:**
  - If any C++ gameplay code accidentally calls raylib graphics routines (e.g. `LoadTexture`, `DrawRectangle`, `GetScreenWidth`) on the dedicated server without going through headless platform stubs (`src/platform/headless/`), the dedicated server will fail to start on headless cloud Linux instances lacking an X11 server or GPU driver.
- **Mitigation Strategy:**
  - Enforce strict link-time separation in `CMakeLists.txt` where `WildboundServer` links only with `raylib_stub.c` and has zero references to OpenGL, GLFW, or X11 symbols.

---

### RSK-002: Live Network Loopback Socket Failure in Cloud Orchestration
- **Category:** Infrastructure & Multiplayer
- **Likelihood:** 4 (High) | **Impact:** 5 (Catastrophic) | **Risk Score:** **20 (CRITICAL)**
- **Description:**
  - As demonstrated in local sandbox runs, multi-process coordination between Dedicated Server (port 17777), Coordinator (port 17780), and Go Backend (port 8080) relies on unrestricted loopback UDP/TCP networking. If containerized (e.g. Docker / Kubernetes), mismatched container network namespaces or blocked ports will silently disconnect game servers from the coordinator fleet.
- **Mitigation Strategy:**
  - Implement health-check probes with explicit retry backoffs, clear connection timeout logging, and unified container network mode (`network_mode: host` or Kubernetes pod sharing).

---

### RSK-003: Memory Fragmentation during Long-Duration Matches
- **Category:** Dedicated Server Stability
- **Likelihood:** 3 (Moderate) | **Impact:** 4 (Major) | **Risk Score:** **12 (MEDIUM)**
- **Description:**
  - Rapid allocation and deallocation of network packets, delta snapshots, Lua temporary tables, and spatial collision queries across a 30-minute match with 32 players could cause heap fragmentation and gradual latency degradation.
- **Mitigation Strategy:**
  - Implement object pooling for snapshots, packet structs, and collision query buffers. Verify peak RSS memory under continuous 60-minute stress tests.

---

### RSK-004: Tiled Map Spatial Collision Scalability
- **Category:** Physics & World Simulation
- **Likelihood:** 3 (Moderate) | **Impact:** 3 (Moderate) | **Risk Score:** **9 (MEDIUM)**
- **Description:**
  - The master island map contains **41,503 collision bounds**. While spatial hash grid indexing (`CollisionGrid`) provides $O(1)$ cell lookups, broad-phase query costs can spike during high-density multi-player melee engagements near complex structures.
- **Mitigation Strategy:**
  - Benchmark spatial index cell sizing ($64 \times 64$ vs $128 \times 128$) and prune redundant collinear collision vertices.

---

### RSK-005: Optimistic Concurrency Conflicts under High Store Traffic
- **Category:** Backend & PostgreSQL
- **Likelihood:** 1 (Low) | **Impact:** 4 (Major) | **Risk Score:** **4 (LOW)**
- **Description:**
  - When players simultaneously purchase and equip cosmetics, the `equipped_loadouts` table utilizes an optimistic locking `version` column. Rapid successive requests from jittery client connections could trigger `409 Conflict` errors if client retry handling is absent.
- **Mitigation Strategy:**
  - Client HTTP layer must implement automatic exponential backoff retry for HTTP 409 responses.

---

### RSK-006: Client Asset Path Resolution across Distribution Packages
- **Category:** Client Packaging & Distribution
- **Likelihood:** 1 (Low) | **Impact:** 2 (Minor) | **Risk Score:** **2 (LOW)**
- **Description:**
  - On different Windows installations (e.g. Steam library vs standalone zip folder), working directory variations may cause `AssetManager::ResolvePath` to fail if invoked with incorrect relative path assumptions.
- **Mitigation Strategy:**
  - Use `PlatformUtils::GetExecutableDirectory()` as the anchor base path rather than relying on current working directory (`cwd`).

