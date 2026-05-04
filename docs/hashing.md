# UEM-256 Hashing

This document describes the UEM-256 hashing algorithm and how it is implemented in `src/encryption.c`.

## Overview

UEM-256 is a lightweight, fixed-size 256-bit string hashing function designed for fast, deterministic text fingerprinting. It is intended for simple hashing and checksum use cases rather than cryptographic security.

## Algorithm Highlights

- Processes input in 8-byte little-endian blocks.
- Uses a small internal state of four 64-bit words.
- Applies mixing rounds with rotations, additions, and XOR operations.
- Finalizes output with padding, length injection, and extra mixing.
- Produces a 32-byte (256-bit) digest.

## C API

The implementation exposes the following functions in `src/encryption.h`:

- `void uem256_init(uem256_ctx *ctx);`
- `void uem256_update(uem256_ctx *ctx, const unsigned char *input, size_t length);`
- `void uem256_final(uem256_ctx *ctx, unsigned char output[32]);`
- `void uem256_hash(const unsigned char *input, size_t length, unsigned char output[32]);`
- `void uem256_hex(const unsigned char digest[32], char hex[65]);`

## Usage Example

In `src/encryption.c`, the `main` function demonstrates how to hash a string and print the result as hexadecimal:

1. Read input from command line or standard input.
2. Compute the digest with `uem256_hash()`.
3. Convert the digest to hex using `uem256_hex()`.
4. Print the hex string.

## Notes

- `uem256_hash()` is a convenience wrapper around `uem256_init()`, `uem256_update()`, and `uem256_final()`.
- The final digest is always 32 bytes, and the hex representation is 64 characters long.
- The algorithm is deterministic and stable across runs.

## When to use UEM-256

Use this hashing method for:

- Quick string identifiers.
- Lightweight checksums.
- Text-based caching keys.
- Non-security string fingerprinting.

> Do not use UEM-256 as a replacement for cryptographic hash functions in security-sensitive applications.
