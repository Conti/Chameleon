/*
 * strntoimax.c
 *
 * strntoimax()
 */

#include <stddef.h>
#include <inttypes.h>

extern uintmax_t strntoumax(const char *nptr, char **endptr, int base, size_t n);

intmax_t strntoimax(const char *nptr, char **endptr, int base, size_t n)
{
	return (intmax_t) strntoumax(nptr, endptr, base, n);
}
