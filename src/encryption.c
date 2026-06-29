/*
 * UEM-256 - Lightweight string hash implemented in C.
 *
 * This is a practical 256-bit string fingerprint function for fast,
 * deterministic hashing of text. It is not intended to replace
 * cryptographic hash functions for security-critical workloads.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "encryption.h"

static inline uint64_t rotl64(uint64_t value, unsigned int shift)
{
    return (value << shift) | (value >> (64 - shift));
}

static inline uint64_t load64_le(const unsigned char *data)
{
    uint64_t result = 0;
    for (unsigned int i = 0; i < 8; ++i)
    {
        result |= (uint64_t)data[i] << (8 * i);
    }
    return result;
}

static inline void store64_le(uint64_t value, unsigned char *output)
{
    for (unsigned int i = 0; i < 8; ++i)
    {
        output[i] = (unsigned char)(value >> (8 * i));
    }
}

static void uem256_mix(uint64_t state[4], uint64_t block)
{
    state[0] ^= block;

    for (int round = 0; round < 2; ++round)
    {
        state[0] += state[1];
        state[1] = rotl64(state[1], 13);
        state[1] ^= state[0];

        state[2] += state[3];
        state[3] = rotl64(state[3], 16);
        state[3] ^= state[2];

        state[0] += state[3];
        state[3] = rotl64(state[3], 21);
        state[3] ^= state[0];

        state[2] += state[1];
        state[1] = rotl64(state[1], 17);
        state[1] ^= state[2];
    }

    state[0] ^= block;
}

void uem256_init(uem256_ctx *ctx)
{
    ctx->state[0] = 0x6d6574615f6d6172ULL;
    ctx->state[1] = 0x7465735f7169735fULL;
    ctx->state[2] = 0x6e69755f4d654f6dULL;
    ctx->state[3] = 0x646f6d5f656e6f6dULL;
    ctx->length = 0;
    ctx->buffer_len = 0;
}

void uem256_update(uem256_ctx *ctx, const unsigned char *input, size_t length)
{
    ctx->length += length;

    if (ctx->buffer_len > 0)
    {
        size_t to_copy = 8 - ctx->buffer_len;
        if (to_copy > length)
            to_copy = length;
        memcpy(ctx->buffer + ctx->buffer_len, input, to_copy);
        ctx->buffer_len += to_copy;
        input += to_copy;
        length -= to_copy;

        if (ctx->buffer_len == 8)
        {
            uint64_t block = load64_le(ctx->buffer);
            uem256_mix(ctx->state, block);
            ctx->buffer_len = 0;
        }
    }

    while (length >= 8)
    {
        uint64_t block = load64_le(input);
        uem256_mix(ctx->state, block);
        input += 8;
        length -= 8;
    }

    if (length > 0)
    {
        memcpy(ctx->buffer, input, length);
        ctx->buffer_len = length;
    }
}

void uem256_final(uem256_ctx *ctx, unsigned char output[32])
{
    unsigned char tail[8] = {0};
    memcpy(tail, ctx->buffer, ctx->buffer_len);
    tail[ctx->buffer_len] = 0x80;
    tail[7] = (unsigned char)ctx->length;
    uem256_mix(ctx->state, load64_le(tail));

    ctx->state[0] ^= (uint64_t)ctx->length;
    for (int i = 0; i < 4; ++i)
    {
        uem256_mix(ctx->state, ctx->state[i] ^ 0xFEDCBA9876543210ULL);
    }

    for (int i = 0; i < 4; ++i)
    {
        store64_le(ctx->state[i], output + 8 * i);
    }
}

void uem256_hash(const unsigned char *input, size_t length, unsigned char output[32])
{
    uem256_ctx ctx;
    uem256_init(&ctx);
    uem256_update(&ctx, input, length);
    uem256_final(&ctx, output);
}

void uem256_hex(const unsigned char digest[32], char hex[65])
{
    static const char digits[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i)
    {
        hex[2 * i] = digits[(digest[i] >> 4) & 0xF];
        hex[2 * i + 1] = digits[digest[i] & 0xF];
    }
    hex[64] = '\0';
}

static uint64_t fnv1a64(const unsigned char *data, size_t length)
{
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < length; ++i)
    {
        hash ^= data[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

static uint64_t djb2(const unsigned char *data, size_t length)
{
    uint64_t hash = 5381ULL;
    for (size_t i = 0; i < length; ++i)
    {
        hash = ((hash << 5) + hash) ^ data[i];
    }
    return hash;
}

static int run_scalar_benchmark(const unsigned char *data, size_t length, size_t iterations, double *elapsed_seconds)
{
    unsigned char digest[32];
    clock_t start = clock();
    for (size_t i = 0; i < iterations; ++i)
    {
        uem256_hash(data, length, digest);
    }
    *elapsed_seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    return 0;
}

static int run_non_crypto_benchmarks(const unsigned char *data, size_t length, size_t iterations, char *buffer, size_t buffer_size)
{
    uint64_t fnv_hash = 0;
    uint64_t djb_hash = 0;
    uint64_t uem_hash = 0;
    double uem_time = 0.0;
    double fnv_time = 0.0;
    double djb_time = 0.0;

    if (buffer == NULL || buffer_size == 0)
        return -1;

    run_scalar_benchmark(data, length, iterations, &uem_time);

    clock_t start = clock();
    for (size_t i = 0; i < iterations; ++i)
    {
        fnv_hash = fnv1a64(data, length);
    }
    fnv_time = (double)(clock() - start) / CLOCKS_PER_SEC;

    start = clock();
    for (size_t i = 0; i < iterations; ++i)
    {
        djb_hash = djb2(data, length);
    }
    djb_time = (double)(clock() - start) / CLOCKS_PER_SEC;

    unsigned char digest[32];
    uem256_hash(data, length, digest);
    uem_hash = ((uint64_t)digest[0]) |
               ((uint64_t)digest[1] << 8) |
               ((uint64_t)digest[2] << 16) |
               ((uint64_t)digest[3] << 24) |
               ((uint64_t)digest[4] << 32) |
               ((uint64_t)digest[5] << 40) |
               ((uint64_t)digest[6] << 48) |
               ((uint64_t)digest[7] << 56);

    int written = snprintf(buffer, buffer_size,
                           "UEM-256 benchmark (%zu iterations, %zu-byte payload):\n"
                           "  UEM-256: %.6f s\n"
                           "  FNV-1a:  %.6f s\n"
                           "  djb2:    %.6f s\n"
                           "  sample hashes: UEM-256=0x%016llx FNV-1a=0x%016llx djb2=0x%016llx\n",
                           iterations,
                           length,
                           uem_time,
                           fnv_time,
                           djb_time,
                           (unsigned long long)uem_hash,
                           (unsigned long long)fnv_hash,
                           (unsigned long long)djb_hash);

    if (written < 0 || (size_t)written >= buffer_size)
        return -1;

    return 0;
}

int uem256_run_benchmark(size_t iterations, size_t data_len, char *buffer, size_t buffer_size)
{
    unsigned char *data = (unsigned char *)malloc(data_len ? data_len : 1);
    if (data == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < data_len; ++i)
    {
        data[i] = (unsigned char)((i * 37 + 11) % 251);
    }

    int rc = run_non_crypto_benchmarks(data, data_len, iterations, buffer, buffer_size);
    free(data);
    return rc;
}
