/*
 * klibc.c
 *
 * glue + initialization
 */

#include "libsaio.h"

int _DefaultRuneLocale;	// todo: fixme

void klibc_start()
{
}

void _exit(int status)
{
    stop("exit() called\n");
    while(1) halt(); // this is never reached
}

char __toupper(char c)
{
	return ((c) & ~32);
}

void __divide_error()
{
	stop("Divide by 0\n");
}

// hack
int
__maskrune(int _c, unsigned long _f)
{
	return 0;
	//return ((_c < 0 || _c >= _CACHED_RUNES) ? ___runetype(_c) :
	//		_CurrentRuneLocale->__runetype[_c]) & _f;
}
