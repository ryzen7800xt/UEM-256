# UEM-256

UEM-256 (Uniform Encryption Method - 256 bits) is a lightweight hashing concept built to turn strings into compact, consistent fingerprints with minimum friction.

It is designed for fast, easy-to-use string hashing with a focus on simplicity, predictable output, and practical results for applications that need a no-nonsense hash function.

## Why UEM-256?

- **Minimal effort:** easy to implement and integrate.
- **Consistent output:** fixed-size hash results for any string length.
- **String-first design:** optimized around text input and common string workflows.
- **Practical utility:** great for quick indexing, simple checksums, and deterministic string transformation.

## What makes it strong?

UEM-256 is built around three core principles:

1. **Speed** — Designed for fast string processing.
2. **Simplicity** — Clear algorithm and easy usage.
3. **Reliability** — Consistent hashing behavior without extra complexity.

## Key goals

- Provide a hashing method that is easy to understand.
- Avoid heavyweight dependencies and overly complex design.
- Make it suitable for rapid prototyping, tooling, and lightweight utility libraries.
- Keep the output deterministic and stable across runs.

## Recommended usage

Use UEM-256 when you need a quick, fixed-size hash for strings and you value simplicity over cryptographic hardness.

### Example scenarios

- Generating compact identifiers for text keys
- Quick collision-resistant checks in non-security contexts
- Lightweight caching keys for strings
- Simple string normalization and hashing pipelines

## Notes

UEM-256 is intended as a practical utility hashing method, not a replacement for cryptographic hash functions in security-sensitive systems.

## C implementation

A reference C implementation is available in `src/encryption.c`.

Build with:

```bash
cc src/encryption.c -o uem256
```

Run:

```bash
./uem256 "hello world"
```

## License

This repository is licensed under the terms described in `LICENSE`.

