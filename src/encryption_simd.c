#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include "encryption.h"

/* Compile-time detection: AVX2 available */
#if defined(__AVX2__) || defined(_M_X64)
#define HAVE_AVX2 1
#else
#define HAVE_AVX2 0
#endif

#ifdef HAVE_AVX2

/* SIMD-optimized rotation for 64-bit values in AVX2 */
static inline __m256i rotl64_avx2(__m256i v, unsigned int shift)
{
    __m256i left = _mm256_slli_epi64(v, shift);
    __m256i right = _mm256_srli_epi64(v, 64 - shift);
    return _mm256_or_si256(left, right);
}

/* Process 64 bytes (4 x 16-byte blocks or 8 x 8-byte blocks) */
static void uem256_mix_simd(__m256i state[4], const unsigned char *blocks)
{
    // Load 64 bytes into 2 x 256-bit registers (32 bytes each)
    __m256i b0 = _mm256_loadu_si256((const __m256i *)blocks);
    __m256i b1 = _mm256_loadu_si256((const __m256i *)(blocks + 32));

    // Initial mix
    state[0] = _mm256_xor_si256(state[0], b0);
    state[1] = _mm256_xor_si256(state[1], b1);

    // Parallel mixing rounds
    for (int round = 0; round < 2; ++round)
    {
        // Round 1
        state[0] = _mm256_add_epi64(state[0], state[1]);
        state[1] = rotl64_avx2(state[1], 13);
        state[1] = _mm256_xor_si256(state[1], state[0]);

        state[2] = _mm256_add_epi64(state[2], state[3]);
        state[3] = rotl64_avx2(state[3], 16);
        state[3] = _mm256_xor_si256(state[3], state[2]);

        // Round 2
        state[0] = _mm256_add_epi64(state[0], state[3]);
        state[3] = rotl64_avx2(state[3], 21);
        state[3] = _mm256_xor_si256(state[3], state[0]);

        state[2] = _mm256_add_epi64(state[2], state[1]);
        state[1] = rotl64_avx2(state[1], 17);
        state[1] = _mm256_xor_si256(state[1], state[2]);
    }

    // Final XOR to make it non-reversible
    state[0] = _mm256_xor_si256(state[0], b0);
    state[1] = _mm256_xor_si256(state[1], b1);
}

void uem256_init_simd(uem256_ctx_simd *ctx)
{
    // Initialize with constants. Using _mm256_set1_epi64x for convenience
    ctx->state[0] = _mm256_set1_epi64x(0x6d6574615f6d6172ULL);
    ctx->state[1] = _mm256_set1_epi64x(0x7465735f7169735fULL);
    ctx->state[2] = _mm256_set1_epi64x(0x6e69755f4d654f6dULL);
    ctx->state[3] = _mm256_set1_epi64x(0x646f6d5f656e6f6dULL);
    ctx->length = 0;
    ctx->buffer_len = 0;
}

void uem256_update_simd(uem256_ctx_simd *ctx, const unsigned char *input, size_t length)
{
    ctx->length += length;

    // Process 64-byte blocks
    while (length >= 64)
    {
        uem256_mix_simd(ctx->state, input);
        input += 64;
        length -= 64;
    }

    // Handle remaining bytes (buffer)
    if (length > 0)
    {
        memcpy(ctx->buffer + ctx->buffer_len, input, length);
        ctx->buffer_len += length;
        if (ctx->buffer_len >= 64)
        {
            // If buffered data reaches 64, mix it
            uem256_mix_simd(ctx->state, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
}

void uem256_final_simd(uem256_ctx_simd *ctx, unsigned char output[32])
{
    // Process remaining buffer with padding if necessary
    if (ctx->buffer_len > 0)
    {
        unsigned char padding[64] = {0};
        memcpy(padding, ctx->buffer, ctx->buffer_len);
        padding[ctx->buffer_len] = 0x80; // Simple padding
        uem256_mix_simd(ctx->state, padding);
    }

    // Extract state to output (Simplified)
    // XOR state registers together to produce 256-bit output
    __m256i final = _mm256_xor_si256(
        _mm256_xor_si256(ctx->state[0], ctx->state[1]),
        _mm256_xor_si256(ctx->state[2], ctx->state[3]));
    _mm256_storeu_si256((__m256i *)output, final);
}

#else

/* Fallback: scalar implementation when AVX2 not available */
void uem256_init_simd(uem256_ctx_simd *ctx)
{
    /* Use standard initialization */
    uem256_ctx temp;
    uem256_init(&temp);
    memcpy(&ctx->scalar_ctx, &temp, sizeof(uem256_ctx));
}

void uem256_update_simd(uem256_ctx_simd *ctx, const unsigned char *input, size_t length)
{
    uem256_update(&ctx->scalar_ctx, input, length);
}

void uem256_final_simd(uem256_ctx_simd *ctx, unsigned char output[32])
{
    uem256_final(&ctx->scalar_ctx, output);
}

#endif

/* Unified high-level hash function */
void uem256_hash_simd(const unsigned char *input, size_t length, unsigned char output[32])
{
    uem256_ctx_simd ctx;
    uem256_init_simd(&ctx);
    uem256_update_simd(&ctx, input, length);
    uem256_final_simd(&ctx, output);
}
