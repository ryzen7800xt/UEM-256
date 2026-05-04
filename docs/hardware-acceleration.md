# Hardware Acceleration Guide

This document explains the hardware-accelerated SIMD implementation of UEM-256.

## Overview

The SIMD-accelerated version (`src/encryption_simd.c`) uses AVX2 (Advanced Vector Extensions 2) instructions to parallelize the mixing operations, processing 4 blocks (32 bytes) in parallel instead of sequentially.

### Performance Benefits

- **4x block parallelism**: Processes 64 bytes per mixing round vs. 8 bytes in scalar version
- **Vector operations**: Uses 256-bit SIMD registers for multiple 64-bit operations simultaneously
- **CPU feature detection**: Automatically falls back to scalar implementation on systems without AVX2

## Building with SIMD Support

### GCC / Clang (Linux, macOS, WSL)

```bash
# Build with AVX2 support
gcc -mavx2 -O3 -o uem256_simd src/encryption.c src/encryption_simd.c
```

### MSVC (Windows)

```bash
cl /arch:AVX2 /O2 src/encryption.c src/encryption_simd.c
```

### Makefile Example

```makefile
CFLAGS = -O3 -std=c99
CFLAGS_SIMD = -mavx2

uem256_simd: src/encryption.c src/encryption_simd.c src/encryption.h
	gcc $(CFLAGS) $(CFLAGS_SIMD) -o uem256_simd src/encryption.c src/encryption_simd.c
```

## API Usage

### Using SIMD Functions

The SIMD API mirrors the scalar API but with `_simd` suffix:

```c
#include "encryption.h"

int main() {
    unsigned char digest[32];
    const unsigned char *text = (unsigned char *)"hello world";
    
    /* Use SIMD-accelerated hash */
    uem256_hash_simd(text, 11, digest);
    
    /* Or use the streaming API */
    uem256_ctx_simd ctx;
    uem256_init_simd(&ctx);
    uem256_update_simd(&ctx, text, 11);
    uem256_final_simd(&ctx, digest);
    
    return 0;
}
```

### Scalar Fallback

If AVX2 is not available at runtime or compile-time:

- The library automatically falls back to the scalar implementation
- No code changes required
- Transparent to the caller

## Technical Details

### AVX2 Implementation

The SIMD mixing function:

1. **Loads** 4 consecutive 8-byte blocks into a 256-bit register
2. **Applies** parallel addition, rotation, and XOR operations
3. **Performs** 2 mixing rounds with 64-bit element-wise operations
4. **Stores** results back to the SIMD state

### Memory Layout

- SIMD state: 4 × 256-bit registers (compatible with `__m256i`)
- Buffer: 64 bytes for unaligned input handling
- Length tracking: Preserved across scalar and SIMD paths

### Instruction Set Requirements

| Target | Instruction Set | Compile Flag |
|--------|-----------------|--------------|
| Modern x86-64 (Haswell+) | AVX2 | `-mavx2` (GCC) or `/arch:AVX2` (MSVC) |
| Older x86-64 (Sandy Bridge) | SSE4.2 | No special flag (default) |
| ARM (Cortex-A72+) | NEON | `-march=native` or specific ARM flag |

## Benchmarking

Compare scalar vs. SIMD performance:

```bash
# Scalar version
time ./uem256 < large_file.txt

# SIMD version (if compiled with -mavx2)
time ./uem256_simd < large_file.txt
```

Expected speedup: **2-4x** for large inputs, depending on CPU and data patterns.

## Future Enhancements

- [ ] SSE4.2 fallback for older CPUs
- [ ] ARM NEON support
- [ ] Streaming hash support for large files
- [ ] Multi-threaded parallel hashing
- [ ] Hardware AES instructions (if applicable)

## Troubleshooting

### "Illegal instruction" error

The binary was built with AVX2 on a CPU that doesn't support it. Compile without `-mavx2` or run on a compatible CPU.

### No performance improvement

Verify compilation with AVX2:

```bash
objdump -d uem256_simd | grep -i "avx"
```

If no AVX2 instructions appear, recompile with proper flags.

### Runtime CPU detection

For production deployments, consider adding runtime CPUID checks to select scalar or SIMD paths dynamically.
