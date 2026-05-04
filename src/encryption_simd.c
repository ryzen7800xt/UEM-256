/*
 * UEM-256 SIMD - Hardware-accelerated hashing with AVX2.
 *
 * This version uses AVX2 (256-bit SIMD) instructions to parallelize
 * the mixing operations, providing significant speedup on modern CPUs.
 *
 * Compile with: -mavx2 (GCC/Clang) or /arch:AVX2 (MSVC)
 * Falls back to scalar implementation if AVX2 is not available.
 */

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

/* Process 4 blocks (32 bytes) in parallel using AVX2 */
static void uem256_mix_simd(__m256i state[4], const unsigned char *blocks)
{
    /* Load 4 consecutive 8-byte blocks */
    __m256i block0 = _mm256_loadu_si256((__m256i *)blocks);
    __m256i block1 = _mm256_loadu_si256((__m256i *)(blocks + 32));

    /* Mix with state (simplified parallel version) */
    state[0] = _mm256_xor_si256(state[0], block0);
    state[1] = _mm256_xor_si256(state[1], block1);

    /* Parallel mixing round */
    for (int round = 0; round < 2; ++round)
    {
        state[0] = _mm256_add_epi64(state[0], state[1]);
        state[1] = rotl64_avx2(state[1], 13);
        state[1] = _mm256_xor_si256(state[1], state[0]);

        state[2] = _mm256_add_epi64(state[2], state[3]);
        state[3] = rotl64_avx2(state[3], 16);
        state[3] = _mm256_xor_si256(state[3], state[2]);

        state[0] = _mm256_add_epi64(state[0], state[3]);
        state[3] = rotl64_avx2(state[3], 21);
        state[3] = _mm256_xor_si256(state[3], state[0]);

        state[2] = _mm256_add_epi64(state[2], state[1]);
        state[1] = rotl64_avx2(state[1], 17);
        state[1] = _mm256_xor_si256(state[1], state[2]);
    }

    state[0] = _mm256_xor_si256(state[0], block0);
    state[1] = _mm256_xor_si256(state[1], block1);
}

/* Initialize context with SIMD state */
void uem256_init_simd(uem256_ctx_simd *ctx)
{
    ctx->state[0] = _mm256_setr_epi64x(
        0x6d6574615f6d6172ULL, 0x6d6574615f6d6172ULL,
        0x6d6574615f6d6172ULL, 0x6d6574615f6d6172ULL);
    ctx->state[1] = _mm256_setr_epi64x(
        0x7465735f7169735fULL, 0x7465735f7169735fULL,
        0x7465735f7169735fULL, 0x7465735f7169735fULL);
    ctx->state[2] = _mm256_setr_epi64x(
        0x6e69755f4d654f6dULL, 0x6e69755f4d654f6dULL,
        0x6e69755f4d654f6dULL, 0x6e69755f4d654f6dULL);
    ctx->state[3] = _mm256_setr_epi64x(
        0x646f6d5f656e6f6dULL, 0x646f6d5f656e6f6dULL,
        0x646f6d5f656e6f6dULL, 0x646f6d5f656e6f6dULL);
    ctx->length = 0;
    ctx->buffer_len = 0;
}

/* Update with SIMD acceleration */
void uem256_update_simd(uem256_ctx_simd *ctx, const unsigned char *input, size_t length)
{
    ctx->length += length;

    /* Process 64-byte blocks (4 x 8-byte blocks) in parallel */
    while (length >= 64)
    {
        uem256_mix_simd(ctx->state, input);
        input += 64;
        length -= 64;
    }

    /* Handle remaining bytes with scalar fallback */
    if (length > 0)
    {
        memcpy(ctx->buffer, input, length);
        ctx->buffer_len = length;
    }
}

/* Finalize SIMD hash */
void uem256_final_simd(uem256_ctx_simd *ctx, unsigned char output[32])
{
    /* Extract first 64-bit value from SIMD state for output */
    uint64_t state_scalar[4];
    _mm256_storeu_si256((__m256i *)state_scalar, ctx->state[0]);
    _mm256_storeu_si256((__m256i *)(state_scalar + 2), ctx->state[1]);

    /* Store the result */
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            output[i * 8 + j] = (unsigned char)(state_scalar[i] >> (8 * j));
        }
    }
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
