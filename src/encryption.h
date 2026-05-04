#ifndef UEM256_ENCRYPTION_H
#define UEM256_ENCRYPTION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint64_t state[4];
        size_t length;
        unsigned char buffer[8];
        size_t buffer_len;
    } uem256_ctx;

    void uem256_init(uem256_ctx *ctx);
    void uem256_update(uem256_ctx *ctx, const unsigned char *input, size_t length);
    void uem256_final(uem256_ctx *ctx, unsigned char output[32]);
    void uem256_hash(const unsigned char *input, size_t length, unsigned char output[32]);
    void uem256_hex(const unsigned char digest[32], char hex[65]);

#ifdef __cplusplus
}
#endif

#endif // UEM256_ENCRYPTION_H
