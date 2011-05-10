/*
 * strtox.c
 *
 * strto...() functions, by macro definition
 */

#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>

extern uintmax_t strntoumax(const char *nptr, char **endptr, int base, size_t n);

TYPE NAME(const char *nptr, char **endptr, int base)
{
	return (TYPE) strntoumax(nptr, endptr, base, ~(size_t) 0);
}
