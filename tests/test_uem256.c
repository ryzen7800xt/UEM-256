#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/encryption.h"

#define CHECK(expr, msg)                        \
    do                                          \
    {                                           \
        if (!(expr))                            \
        {                                       \
            fprintf(stderr, "FAIL: %s\n", msg); \
            exit(EXIT_FAILURE);                 \
        }                                       \
    } while (0)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static void to_hex(const unsigned char digest[32], char hex[65])
{
    uem256_hex(digest, hex);
}

static void check_digest_equal(const unsigned char *a, const unsigned char *b, const char *label)
{
    if (memcmp(a, b, 32) != 0)
    {
        char hex_a[65], hex_b[65];
        to_hex(a, hex_a);
        to_hex(b, hex_b);
        fprintf(stderr, "FAIL: %s\n  expected: %s\n  actual:   %s\n", label, hex_a, hex_b);
        exit(EXIT_FAILURE);
    }
}

static void check_digest_not_equal(const unsigned char *a, const unsigned char *b, const char *label)
{
    if (memcmp(a, b, 32) == 0)
    {
        char hex[65];
        to_hex(a, hex);
        fprintf(stderr, "FAIL: %s - digests unexpectedly equal: %s\n", label, hex);
        exit(EXIT_FAILURE);
    }
}

static void test_known_vectors(void)
{
    struct
    {
        const char *input;
        const char *expected_hex;
    } vectors[] = {
        {"", "ff524be2b601b4c42fd991416c21387105df2a0436c7bb2981481b5e4b44f0a9"},
        {"a", "bb32eb5d34a3fcc5636f144012382083c49543d46c34fa9755225c4f063d0cdf"},
        {"hello world", "aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375"},
        {"UEM-256", "eb96958ffd15ab17f2f8af60f499cce94a82ff52eac1f3178256e20f643e5cb8"},
    };

    for (size_t i = 0; i < ARRAY_SIZE(vectors); ++i)
    {
        unsigned char digest[32];
        char hex[65];
        uem256_hash((const unsigned char *)vectors[i].input, strlen(vectors[i].input), digest);
        uem256_hex(digest, hex);
        CHECK(strcmp(hex, vectors[i].expected_hex) == 0, vectors[i].input);
    }
}

static void test_update_final_equivalence(void)
{
    const unsigned char data[] = "The quick brown fox jumps over the lazy dog";
    const size_t length = strlen((const char *)data);
    unsigned char baseline[32];
    uem256_hash(data, length, baseline);

    for (size_t chunk = 1; chunk <= length; ++chunk)
    {
        uem256_ctx ctx;
        unsigned char digest[32];
        uem256_init(&ctx);

        size_t offset = 0;
        while (offset < length)
        {
            size_t size = chunk;
            if (offset + size > length)
                size = length - offset;
            uem256_update(&ctx, data + offset, size);
            offset += size;
        }

        uem256_final(&ctx, digest);
        check_digest_equal(baseline, digest, "update/final equivalence");
    }
}

static void test_zero_length_update(void)
{
    const unsigned char data[] = "buffering test";
    const size_t length = strlen((const char *)data);
    unsigned char baseline[32];
    uem256_hash(data, length, baseline);

    uem256_ctx ctx;
    unsigned char digest[32];
    uem256_init(&ctx);
    uem256_update(&ctx, data, 3);
    uem256_update(&ctx, data + 3, 0);
    uem256_update(&ctx, data + 3, length - 3);
    uem256_final(&ctx, digest);

    check_digest_equal(baseline, digest, "zero-length update");
}

static void test_padding_and_length(void)
{
    const unsigned char a[] = "abc";
    const unsigned char b[] = "abc\0";
    unsigned char digest_a[32];
    unsigned char digest_b[32];

    uem256_hash(a, 3, digest_a);
    uem256_hash(b, 4, digest_b);
    check_digest_not_equal(digest_a, digest_b, "length-sensitive padding");
}

static void test_hex_format(void)
{
    unsigned char digest[32] = {0};
    char hex[65];
    uem256_hex(digest, hex);

    CHECK(strlen(hex) == 64, "hex output length");
    for (size_t i = 0; i < 64; ++i)
    {
        const char ch = hex[i];
        CHECK((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'), "hex format");
    }
    CHECK(hex[64] == '\0', "hex null terminator");
}

static void test_distinctness(void)
{
    const size_t count = 48;
    unsigned char digs[count][32];
    unsigned char buffer[64];

    for (size_t i = 0; i < count; ++i)
    {
        size_t len = 32;
        for (size_t j = 0; j < len; ++j)
            buffer[j] = (unsigned char)((i * 37 + j * 17) % 251 + 1);

        uem256_hash(buffer, len, digs[i]);
    }

    for (size_t i = 0; i < count; ++i)
    {
        for (size_t j = i + 1; j < count; ++j)
        {
            check_digest_not_equal(digs[i], digs[j], "distinctness collision check");
        }
    }
}

static void test_buffer_boundaries(void)
{
    const unsigned char data[] = "123456789ABCDEF";
    const size_t length = strlen((const char *)data);
    unsigned char baseline[32];
    uem256_hash(data, length, baseline);

    uem256_ctx ctx;
    unsigned char digest[32];
    uem256_init(&ctx);
    uem256_update(&ctx, data, 7);
    uem256_update(&ctx, data + 7, 1);
    uem256_update(&ctx, data + 8, length - 8);
    uem256_final(&ctx, digest);

    check_digest_equal(baseline, digest, "boundary split updates");
}

static void test_benchmark_report(void)
{
    char report[512];
    int rc = uem256_run_benchmark(8, 256, report, sizeof(report));
    CHECK(rc == 0, "benchmark function");
    CHECK(strstr(report, "UEM-256") != NULL, "benchmark report contains UEM-256");
    CHECK(strstr(report, "FNV-1a") != NULL, "benchmark report contains FNV-1a");
    CHECK(strstr(report, "djb2") != NULL, "benchmark report contains djb2");
}

int main(void)
{
    printf("Running UEM-256 comprehensive tests...\n");
    test_known_vectors();
    test_update_final_equivalence();
    test_zero_length_update();
    test_padding_and_length();
    test_hex_format();
    test_distinctness();
    test_buffer_boundaries();
    test_benchmark_report();
    printf("PASS: all tests completed successfully.\n");
    return EXIT_SUCCESS;
}
