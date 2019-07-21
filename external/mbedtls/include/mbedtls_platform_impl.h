
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *mbedtls_platform_calloc(size_t n, size_t s);
void mbedtls_platform_free(void *p);

#ifdef __cplusplus
};
#endif
