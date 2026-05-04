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

int main(int argc, char *argv[])
{
    const char *text = NULL;
    char stack_buffer[4096];

    if (argc >= 2)
    {
        text = argv[1];
    }
    else
    {
        if (!fgets(stack_buffer, sizeof(stack_buffer), stdin))
        {
            return 1;
        }
        size_t line_length = strcspn(stack_buffer, "\r\n");
        stack_buffer[line_length] = '\0';
        text = stack_buffer;
    }

    unsigned char digest[32];
    char hex[65];
    uem256_hash((const unsigned char *)text, strlen(text), digest);
    uem256_hex(digest, hex);

    printf("%s  %s\n", hex, text);
    return 0;
}
