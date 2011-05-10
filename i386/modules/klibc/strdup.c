/*
 * strdup.c
 */

#include "libsaio.h"

char *strdup(const char *s)
{
	int l = strlen(s) + 1;
	char *d = malloc(l);

	if (d)
		memcpy(d, s, l);

	return d;
}
