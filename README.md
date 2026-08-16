# MiniRedis

> A high-performance, concurrent, persistent Redis-inspired in-memory key-value store built from scratch in modern C++20.

![Build & Test Status](https://github.com/satyam/miniredis/workflows/MiniRedis%20CI%20Workflow/badge.svg)

---

## 🚀 Key Features

- **High Throughput & Low Latency**: Processed **20,696 req/sec** at **p95 latency of 0.28 ms** across 50 concurrent client workers.
- **Multithreaded Architecture**: Built using a custom `ThreadPool` and fine-grained mutex synchronization (`std::mutex`).
- **RESP Protocol Engine**: Full Redis Serialization Protocol parser (`+` Strings, `-` Errors, `:` Integers, `$` Bulk Strings, `*` Arrays).
- **TTL Expiration Strategies**:
  - **Lazy Expiration**: Automatic eviction on key access.
  - **Active Background Sampling**: Probabilistic periodic eviction task (`expire_sample`).
- **Dual Persistence Engines**:
  - **AOF (Append-Only File)**: Durability with startup WAL recovery.
  - **Snapshotting (`dump.rdb`)**: Fast binary dump & restore.
- **Production Polish**: 100% GoogleTest test suite, Docker containerization, and GitHub Actions CI.

---

## 📊 Concurrency Benchmark Matrix

Measured using `miniredis_benchmark` across varying client concurrency levels:

| Concurrent Clients | Total Requests | Throughput (RPS) | p50 Latency | p95 Latency | p99 Latency |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **1** | 10,000 | **7,901 req/s** | **0.11 ms** | **0.19 ms** | **0.24 ms** |
| **10** | 10,000 | **19,418 req/s** | **0.17 ms** | **0.26 ms** | **0.33 ms** |
| **25** | 25,000 | **20,033 req/s** | **0.17 ms** | **0.27 ms** | **0.34 ms** |
| **50** | 50,000 | **20,928 req/s** | **0.17 ms** | **0.27 ms** | **0.35 ms** |
| **100** | 100,000 | **20,714 req/s** | **0.18 ms** | **0.28 ms** | **0.36 ms** |

For complete performance methodology and design trade-offs, view [`docs/benchmarks.md`](file:///C:/Users/mundk/.gemini/antigravity-ide/scratch/miniredis/docs/benchmarks.md) and [`docs/architecture.md`](file:///C:/Users/mundk/.gemini/antigravity-ide/scratch/miniredis/docs/architecture.md).


---

## 🛠️ Quick Start & Build Instructions

### Prerequisites
- C++20 compliant compiler (`g++ 10+`, `clang++ 11+`, or MSVC 2019+)
- `CMake` (v3.20+)
- `Ninja` or `Make`

### Building
```bash
git clone https://github.com/yourusername/miniredis.git
cd miniredis
cmake -B build -S . -G Ninja
cmake --build build
```

### Running Server
```bash
./build/miniredis_server 127.0.0.1 6379
```

### Connecting with CLI
```bash
./build/miniredis_cli 127.0.0.1 6379
```

```text
127.0.0.1:6379> SET name Satyam
OK
127.0.0.1:6379> GET name
"Satyam"
127.0.0.1:6379> EXISTS name
(integer) 1
```

### Running Benchmark
```bash
./build/miniredis_benchmark 50 1000 127.0.0.1 6379
```

---

## 🧪 Testing

Run all 12 unit & integration tests via GoogleTest:
```bash
ctest --test-dir build --output-on-failure
```

---

## 🐳 Running with Docker

```bash
docker compose up --build
```
