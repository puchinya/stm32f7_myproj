
#include "mbedtls_platform_impl.h"
#include <kos.h>
#include <string.h>
#include <kfw_system.hpp>
#include <timing_alt.h>

void *mbedtls_platform_calloc(size_t n, size_t s)
{
	size_t total_size = n *s;
	void *p = kos_alloc(n * s);
	if(!p) return 0;
	memset(p, 0, total_size);

	return p;
}

void mbedtls_platform_free(void *p)
{
	return kos_free(p);
}

int mbedtls_platform_entropy_poll( void *data, unsigned char *output, size_t len,
                           size_t *olen )
{
	*olen = len;
	return 0;
}

unsigned long mbedtls_timing_hardclock()
{
	return kfw::kfw_system_get_cycle_counter();
}

