#include "libsaio.h"

void uClibcxx_start()
{
}

void abort()
{
	stop("uClibc+: abort()\n");
}


