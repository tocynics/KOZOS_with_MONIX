/* $Id: string.c,v 1.2.2.5 2007/01/29 23:07:47 kuwa Exp $ */
/*
 * Copyright (C) 2005-2007 Shuji KUWAHARA. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <ctype.h>	/* for str*casecmp */

size_t strlen(const char *s)
{
	size_t len = 0;

	while (*s++)
		len++;
	return len;
}

char *strcpy(char *dst, const char *src)
{
	char *save = dst;

	while ((*dst++ = *src++) != 0)
		;
	return save;
}

void *memset(void *dst, int c, size_t len)
{
	unsigned char *p = dst;

	while (len-- > 0)
		*p++ = c;
	return dst;
}

void *memcpy(void *dst, const void *src, size_t len)
{
	unsigned char *d = dst;
	const unsigned char *s = src;

	while (len-- > 0)
		*d++ = *s++;
	return dst;
}

char *strcat(char *s, const char *append)
{
	char *p = s;

	while (*p)
		p++;
	while ((*p++ = *append++) != 0)
		;
	return s;
}

int strncmp(const char *s1, const char *s2, size_t len)
{
	if (len > 0) {
		while (*s1 == *s2) {
			if (*s1 == '\0')
				return 0;
			if (--len == 0)
				break;
			s1++;
			s2++;
		}
		return ((unsigned char)*s1 - (unsigned char)*s2);
	}
	return 0;
}

#if 0
char *strchr(const char *s, int c)
{
	while (*s) {
		if (*s == c)
			break;
		s++;
	}
	return (char *)s;
}

char *strncpy(char *dst, const char *src, size_t len)
{
	char *p = dst;

	if (len > 0)
		while ((*p++ = *src++) != 0)
			if (--len == 0) {
				*p = '\0';
				break;
			}
	return dst;
}

char *strncat(char *s, const char *append, size_t count)
{
	char *save = s;

	if (count > 0) {
		while (*s)
			s++;
		while ((*s++ = *append++) != 0)
			if (--count == 0) {
				*s = '\0';
				break;
			}
	}
	return save;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2) {
		if (*s1 == '\0')
			return 0;
		s1++;
		s2++;
	}
	return ((unsigned char)*s1 - (unsigned char)*s2);
}

int strcasecmp(const char *s1, const char *s2)
{
	while (tolower(*s1) == tolower(*s2)) {
		if (*s1 == '\0')
			return 0;
		s1++;
		s2++;
	}
	return ((unsigned char)tolower(*s1) - (unsigned char)tolower(*s2));
}

int strncasecmp(const char *s1, const char *s2, size_t len)
{
	if (len > 0) {
		while (tolower(*s1) == tolower(*s2)) {
			if (*s1 == '\0')
				return 0;
			if (--len == 0)
				break;
			s1++;
			s2++;
		}
		return (unsigned char)tolower(*s1) -
		  (unsigned char)tolower(*s2);
	}
	return 0;
}

int memcmp(const void *b1, const void *b2, size_t len)
{
	const unsigned char *p1 = b1;
	const unsigned char *p2 = b2;

	if (len == 0)
		return 0;
	while (len-- > 0) {
		if (*p1 != *p2)
			return *p1 - *p2;
		p1++;
		p2++;
	}
	return 0;
}

void *memmove(void *dst, const void *src, size_t len)
{
	unsigned char *d = dst;
	const unsigned char *s = src;

	if (len == 0 || dst == src)
		return dst;
	if (dst > src && src + len < dst) {
		/* copy backward */
		d += len;
		s += len;
		while (len-- > 0)
			*(--d) = *(--s);
	} else
		/* copy forward */
		while (len-- > 0)
			*d++ = *s++;
	return dst;
}

char *strrchr(const char *sp, int c)
{
	char *r;
	r = (char *)NULL;
	do
		if (*sp == c)
			r = (char *)sp;
	while (*sp++);
	return r;
}

void *memchr(const void *buf, int c, size_t n)
{
	char *p = (char *)buf;
	while (n-- > 0) {
		if (*p == c)
			return p;
		p++;
	}
	return NULL;
}
#endif
