# UEM-256

**Uniform Encryption Method • 256-bit**

<p align="center">
  <img src="https://img.shields.io/badge/status-experimental-111111?style=for-the-badge">
  <img src="https://img.shields.io/badge/scope-lightweight-222222?style=for-the-badge">
  <img src="https://img.shields.io/badge/output-256--bit-333333?style=for-the-badge">
  <img src="https://img.shields.io/badge/license-MIT-444444?style=for-the-badge">
</p>

<p align="center">
  <b>Fast. Deterministic. Minimal.</b><br>
  A no-friction string hashing method built for real-world utility.
</p>

---

## What is UEM-256?

UEM-256 is a compact hashing method that converts strings into fixed 256-bit fingerprints.

It is designed for developers who want something simple, predictable, and fast without pulling in heavy cryptographic systems.

> Same input in. Same output out. Every time.

---

## Why it exists

Most hashing tools fall into two extremes:

* Overly complex cryptographic systems
* Weak ad hoc string transformations

UEM-256 sits in the middle.

It gives you structure without overhead and consistency without complexity.

---

## Core Principles

### Speed

Processes strings quickly with minimal computation.

### Simplicity

Readable logic. Easy to implement. Easy to port.

### Determinism

No randomness. No state. Fully predictable output.

---

## Properties

| Property      | Value              |
| ------------- | ------------------ |
| Output size   | 256-bit (32 bytes) |
| Input type    | Strings            |
| Deterministic | Yes                |
| Stateful      | No                 |
| Dependencies  | None               |

---

## Where it shines

UEM-256 is useful when you need fast, stable hashing in non-security contexts.

### Example uses

* String ID generation
* Hash maps and indexing
* Cache keys
* Deduplication pipelines
* Deterministic transforms

---

## Where it does NOT belong

Do not use UEM-256 for:

* Password hashing
* Encryption
* Digital signatures
* Anything security critical

If an attacker is part of your threat model, use a proven cryptographic hash.

---

## Quick start

### Build

```bash
cc src/encryption.c -o uem256
```

### Run

```bash
./uem256 "hello world"
```

### Cross-platform runners

macOS / Linux:

```bash
./tests/run_tests.sh
```

Windows PowerShell:

```powershell
tests\run_tests.ps1
```

If you have GNU Make installed, use:

```bash
make test
```

You can also run:

```bash
make lint
make fmt
```

For full platform-specific instructions, see `docs/cross-platform.md`.

---

## Visual flow

```
input string
     ↓
[ UEM-256 ]
     ↓
256-bit fingerprint
```

---

## Design goals

* Keep hashing understandable
* Keep implementation small
* Keep output consistent
* Avoid unnecessary abstraction

---

## Reference implementation

Located at:

```
src/encryption.c
```

Minimal. Direct. Transparent.

---

## License

MIT License

You are free to use, modify, and distribute this software with attribution.