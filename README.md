# durable-task-engine

A concurrent task engine in C++17. Right now it's just the in-memory part — a
bounded queue and a pool of worker threads. Persistence is next.

![CI](https://github.com/paytonyu/durable-task-engine/actions/workflows/ci.yml/badge.svg)

You hand it callables and the workers run them. The queue holds a fixed number of
items, so if you submit faster than the workers keep up, your push blocks instead
of the queue growing forever. Shutdown finishes what's already queued.

## Why I built it this way

A bounded queue because an unbounded one just turns "producer is faster than
consumer" into "we run out of memory." Blocking the producer at least puts the
problem where you can see it.

Two condition variables, one for producers waiting on space and one for consumers
waiting on items — with a single one you wake threads that can't do anything.
Every wait re-checks its condition in a loop, since a wakeup doesn't mean the
condition is true.

Shutdown drains rather than dropping: consumers keep popping until the queue is
empty *and* the stop flag is set, so it refuses new work but finishes what it
accepted. Abort-immediately would be the same code with the two checks in `pop()`
swapped.

Mutex and condvars, not lock-free. I don't think the lock is the bottleneck at
this size, but I haven't benchmarked it, so that's on the roadmap.

## Tests

GoogleTest under ThreadSanitizer. The main one is 4 producers pushing 25,000
items each through a 64-slot queue with 4 consumers — every value has to show up
exactly once, and the queue is small on purpose so producers hit the blocking
path. Others cover shutdown draining, pushes after shutdown getting rejected,
blocked consumers waking up, and a throwing task not killing its worker.

CI runs on Linux every push. That's how I found a missing include that built fine
on my Mac because libc++ pulls it in transitively.

## Build

```
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

ThreadSanitizer:

```
cmake -B build-tsan -DCMAKE_CXX_FLAGS="-fsanitize=thread -g -O1"
cmake --build build-tsan
./build-tsan/unit_tests
```

## Roadmap

No durability yet. Plan: SQLite in WAL mode, tasks written to disk before they
enter the queue and tracked through PENDING → RUNNING → COMPLETED/FAILED. Crash
recovery re-queues anything left in RUNNING at startup — that's at-least-once,
not exactly-once, since a worker can finish and die before recording it, so
duplicates get handled by a primary key on the results table instead of two-phase
commit. Then fault injection (SIGKILL at random points, restart, check nothing's
lost), benchmarks for throughput and p50/p95/p99 across 1–16 workers, and futures
via `std::packaged_task` so callers can wait on results.
