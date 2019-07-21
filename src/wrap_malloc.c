
#include <itron.h>
#include <stddef.h>

void *__wrap_malloc(size_t s)
{
	return kos_alloc(s);
}

void *__wrap_calloc(size_t s, size_t n)
{
	return kos_alloc(s * n);
}

void __wrap_free(void *p)
{
	kos_free(p);
}

void *__wrap_realloc(void *p, size_t s)
{
	return 0;
}
