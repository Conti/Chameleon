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
/* string operations */

#include "libsa.h"

void * memset(void * dst, int val, size_t len)
{
    asm volatile ( "rep; stosb"
       : "=c" (len), "=D" (dst)
       : "0" (len), "1" (dst), "a" (val)
       : "memory" );

    return dst;
}

#if 0
void * memcpy(void * dst, const void * src, size_t len)
{
    asm volatile ( "rep; movsb"
       : "=c" (len), "=D" (dst), "=S" (src)
       : "0" (len), "1" (dst), "2" (src)
       : "memory" );

    return dst;
}

void bcopy(const void * src, void * dst, size_t len)
{
	memcpy(dst, src, len);
}

void bzero(void * dst, size_t len)
{
    memset(dst, 0, len);
}

#else
void * memcpy(void * dst, const void * src, size_t len)
{
    asm volatile ( "cld                  \n\t"
         "movl %%ecx, %%edx    \n\t"
         "shrl $2, %%ecx       \n\t"
         "rep; movsl           \n\t"
         "movl %%edx, %%ecx    \n\t"
         "andl $3, %%ecx       \n\t"
         "rep; movsb           \n\t"
       : "=D" (dst)
       : "c" (len), "D" (dst), "S" (src)
       : "memory", "%edx" );

    return dst;
}

void bcopy(const void * src, void * dst, size_t len)
{
    asm volatile ( "cld                  \n\t"
         "movl %%ecx, %%edx    \n\t"
         "shrl $2, %%ecx       \n\t"
         "rep; movsl           \n\t"
         "movl %%edx, %%ecx    \n\t"
         "andl $3, %%ecx       \n\t"
         "rep; movsb           \n\t"
       :
       : "c" (len), "D" (dst), "S" (src)
       : "memory", "%edx" );
}

void bzero(void * dst, size_t len)
{
    asm volatile ( "xorl %%eax, %%eax    \n\t"
         "cld                  \n\t"
         "movl %%ecx, %%edx    \n\t"
         "shrl $2, %%ecx       \n\t"
         "rep; stosl           \n\t"
         "movl %%edx, %%ecx    \n\t"
         "andl $3, %%ecx       \n\t"
         "rep; stosb           \n\t"
       : 
       : "c" (len), "D" (dst)
       : "memory", "%eax" );
}
#endif

/* #if DONT_USE_GCC_BUILT_IN_STRLEN */

#define tolower(c)     ((int)((c) & ~0x20))
#define toupper(c)     ((int)((c) | 0x20))

size_t strlen(const char * s)
{
  const char* save = s;
  while (*s++);
  return (--s) - save;
}

/*#endif*/

/* NOTE: Moved from ntfs.c */
int
memcmp(const void *p1, const void *p2, size_t len)
{
    while (len--) {
        if (*(const char*)(p1++) != *(const char*)(p2++))
            return -1;
    }
    return 0;
}

int
strcmp(const char * s1, const char * s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}

/* Derived from FreeBSD source */
int strncmp(const char * s1, const char * s2, size_t n)
{
  if (!n)
    return 0;
  do {
    if (*s1 != *s2++)
      return (*(const unsigned char *)s1 -
              *(const unsigned char *)(s2 - 1));
    if (!*s1++)
      break;
  } while (--n);
  return 0;
}

char *
strcpy(char * s1, const char * s2)
{
	register char *ret = s1;
	while ((*s1++ = *s2++))
		continue;
	return ret;
}

char *
stpcpy(char * s1, const char * s2)
{
	while ((*s1++ = *s2++)) {
		continue;
	}
	return --s1;
}

char *
strncpy(char * s1, const char * s2, size_t n)
{
	register char *ret = s1;
	while (n && (*s1++ = *s2++))
      --n;
	if (n > 0) {
		bzero(s1, n);
	}
	return ret;
}

char *
stpncpy(char * s1, const char * s2, size_t n)
{
	while (n && (*s1++ = *s2++))
      --n;
	if (n > 0)
      bzero(s1, n);
    return s1;
}

char *
strstr(const char *in, const char *str)
{
    char c;
    size_t len;

    c = *str++;
    if (!c)
        return (char *) in;	// Trivial empty string case

    len = strlen(str);
    do {
        char sc;

        do {
            sc = *in++;
            if (!sc)
                return (char *) 0;
        } while (sc != c);
    } while (strncmp(in, str, len) != 0);

    return (char *) (in - 1);
}

int
ptol(const char *str)
{
	register int c = *str;

	if (c <= '7' && c >= '0')
		c -= '0';
	else if (c <= 'h' && c >= 'a')
		c -= 'a';
	else c = 0;
	return c;
}

int
atoi(const char *str)
{
	register int sum = 0;
	while (*str == ' ' || *str == '\t')
		str++;
	while (*str >= '0' && *str <= '9') {
		sum *= 10;
		sum += *str++ - '0';
	}
	return sum;
}

char *strncat(char *s1, const char *s2, size_t n)
{
	register char *ret = s1;
	while (*s1)
		s1++;
	while (n-- && (*s1++ = *s2++));
	return ret;
}

char *strcat(char *s1, const char *s2)
{
	register char *ret = s1;
	while (*s1)
		s1++;
	while ((*s1++ = *s2++));
	return ret;
}

char *strdup(const char *s1)
{
	return strcpy(malloc(strlen(s1) + 1), s1);
}

#if STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t len)
{
	register int n = len;
	while (--n >= 0 && tolower(*s1) == tolower(*s2++))
		if (*s1++ == '\0')
			return(0);
	return(n<0 ? 0 : tolower(*s1) - tolower(*--s2));
}
#endif

char* strchr(const char *str, int c)
{
    do
    {
        if(*str == c)
            return (char*)str;
    }
    while(*(str++));
    
    return 0;
}        
        
char* strbreak(const char *str, char **next, long *len)
{
    char *start = (char*)str, *end;
    bool quoted = false;
    
    if ( !start || !len )
        return 0;
    
    *len = 0;
    
    while ( isspace(*start) )
        start++;
    
    if (*start == '"')
    {
        start++;

        end = strchr(start, '"');
        if(end)
            quoted = true;
        else
            end = strchr(start, '\0');
    }
    else
    {
        for ( end = start; *end && !isspace(*end); end++ )
        {}
    }

    *len = end - start;

    if(next)
        *next = quoted ? end+1 : end;

    return start;
}

/* COPYRIGHT NOTICE: checksum8 from AppleSMBIOS */
uint8_t checksum8( void * start, unsigned int length )
{
    uint8_t   csum = 0;
    uint8_t * cp = (uint8_t *) start;
    unsigned int i;

	for ( i = 0; i < length; i++) {
		csum += *cp++;
	}
	return csum;
}

