/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __BOOT_LIBSA_H
#define __BOOT_LIBSA_H

/* Exported API for standalone library */

#include <mach-o/loader.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * ctype stuff (aserebln)
 */
static inline int isupper(int c)
{
	return (c >= 'A' && c <= 'Z');
}

static inline int islower(int c)
{
	return (c >= 'a' && c <= 'z');
}

static inline int isalpha(int c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

static inline int isascii(int c)
{
	return ( (c >= 0x20) && (c < 0x7f) );
}

static inline int isspace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\12');
}

static inline int isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

static inline int isxdigit(int c)
{
	return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

static inline int ispunct(int c)
{
	return (c == '.' || c == '-');
}

/*
 * string.c
 */
#ifndef bcopy
extern void bcopy(const void * src, void * dst, size_t len);
#endif

#ifndef bzero
extern void bzero(void * dst, size_t len);
#else
#error bzero is defined.
#endif

extern void * memset(void * dst, int c, size_t n);
extern void * memcpy(void * dst, const void * src, size_t len);
extern int    memcmp(const void * p1, const void * p2, size_t len);
extern int    strcmp(const char * s1, const char * s2);
extern int    strncmp(const char * s1, const char * s2, size_t n);
extern char * strcpy(char * s1, const char * s2);
extern char * stpcpy(char * s1, const char * s2);
extern char * strncpy(char * s1, const char * s2, size_t n);
extern char * strpcpy(char * s1, const char * s2, size_t n);
extern size_t strlcpy(char * s1, const char * s2, size_t n);
extern char * strstr(const char *in, const char *str);
extern int    atoi(const char * str);
extern int    ptol(const char * str);
extern size_t    strlen(const char * str);
extern char * strcat(char * s1, const char * s2);
extern char * strncat(char * s1, const char * s2, size_t n);
extern char * strdup(const char *s1);

#if STRNCASECMP
extern int strncasecmp(const char *s1, const char *s2, size_t n);
#endif

extern char * strchr(const char *str, int c);
extern char * strbreak(const char *str, char **next, long *len);

extern uint8_t checksum8( void * start, unsigned int length );

/*
 * error.c
 */
extern int    errno;
extern char * strerror(int errnum);

/*
 * strtol.c
 */
extern long strtol(const char * nptr, char ** endptr, int base);
extern unsigned long strtoul(const char * nptr, char ** endptr, int base);
extern unsigned long long strtouq(const char *nptr, char ** endptr, int base);

/*
 * prf.c
 */
extern void prf(const char * fmt, va_list ap, int (*putfn_p)(), void * putfn_arg);

/*
 * printf.c
 */
extern int sprintf(char *s, const char * format, ...);
extern int snprintf(char *s, size_t size, const char * format, ...);
extern int slvprintf(char * buffer, int len, const char * fmt, va_list arg);

/*
 * zalloc.c
 */
#define malloc(size) safe_malloc(size, __FILE__, __LINE__)
extern void   malloc_init(char * start, int size, int nodes, void (*malloc_error)(char *, size_t, const char *, int));
extern void * safe_malloc(size_t size,const char *file, int line);
extern void   free(void * start);
extern void * realloc(void * ptr, size_t size);

/*
 * getsegbyname.c
 */
extern struct segment_command * getsegbynamefromheader(struct mach_header * mhp, char * segname);

#endif /* !__BOOT_LIBSA_H */
