#ifndef UEM256_ENCRYPTION_H
#define UEM256_ENCRYPTION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void uem256_hash(const unsigned char *input, size_t length, unsigned char output[32]);
    void uem256_hex(const unsigned char digest[32], char hex[65]);

#ifdef __cplusplus
}
#endif

#endif // UEM256_ENCRYPTION_H
