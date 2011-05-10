/*
 * sscanf()
 */

#include "libsaio.h"
extern int vsscanf(const char *buffer, const char *format, va_list ap);
int sscanf(const char *str, const char *format, ...)
{
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = vsscanf(str, format, ap);
	va_end(ap);

	return rv;
}
