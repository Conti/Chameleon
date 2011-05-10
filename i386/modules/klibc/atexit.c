/*
 * atexit.c
 */

#include <stdlib.h>

int atexit(void (*fctn) (void))
{
	return 0;
}

int __cxa_atexit(void (*fctn) (void))
{
	return 0;
	//return on_exit((void (*)(int, void *))fctn, NULL);
}
