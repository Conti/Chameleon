/*
 * atox.c
 *
 * atoi(), atol(), atoll()
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

extern uintmax_t strntoumax(const char *nptr, char **endptr, int base, size_t n);

TYPE NAME(const char *nptr)
{
	return (TYPE) strntoumax(nptr, (char **)NULL, 10, ~(size_t) 0);
}
