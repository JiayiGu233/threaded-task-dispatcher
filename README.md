# Threaded Task Dispatcher

[![CI](https://github.com/JiayiGu233/threaded-task-dispatcher/actions/workflows/ci.yml/badge.svg)](https://github.com/JiayiGu233/threaded-task-dispatcher/actions/workflows/ci.yml)

**C · POSIX threads · Condition variables**

A batched producer–consumer task processor with five workers, SHA-256 work items and orderly shutdown.

## Quick start

```text
make
./bin/hogwarts 100
```

## Design

One producer posts batches of up to ten tasks. Workers dequeue under a mutex, process outside the critical section and signal when the queue drains. Shutdown is cooperative; joining workers waits for in-flight work.

## Scope and validation

Requires OpenSSL development headers. The portfolio edition replaces the original cancellation/semaphore shutdown with condition variables, addressing empty-workload races and shutdown hangs.

See [VALIDATION.md](VALIDATION.md) for checks run against this edition. Source background and authorship are recorded in [PROVENANCE.md](PROVENANCE.md).
