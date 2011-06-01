MODULE_NAME = klibc
MODULE_DESCRIPTION = This module provides a standard c library for modules to link to if the library provided by chameleon is insufficient. This is currently only used by the uClibc++ library.
MODULE_AUTHOR = 
MODULE_VERSION = "1.5.20"
MODULE_COMPAT_VERSION = "1.5.20"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = klibc

MODULE_OBJS = 	klibc.o \
		__ashldi3.o __ashrdi3.o __clzsi2.o __divdi3.o __divsi3.o \
		__lshrdi3.o __moddi3.o __modsi3.o __udivdi3.o \
		__udivmoddi4.o __udivmodsi4.o __udivsi3.o \
		__umoddi3.o __umodsi3.o \
		strntoumax.o strntoimax.o atol.o atoll.o \
		strcasecmp.o strncasecmp.o strlcat.o strndup.o strnlen.o \
		strsep.o strtoimax.o strtok_r.o strtok.o strtol.o strtoll.o strtotimespec.o strtotimeval.o \
		strtoul.o strtoull.o strtoumax.o strxspn.o strpbrk.o \
		bsearch.o calloc.o \
		jrand48.o lrand48.o mrand48.o srand48.o nrand48.o seed48.o \
		memccpy.o memchr.o memmem.o memmove.o memrchr.o memswap.o  \
		qsort.o sha1hash.o onexit.o atexit.o exit.o \
		snprintf.o vsnprintf.o sscanf.o vsscanf.o\

include ../MakeInc.dir