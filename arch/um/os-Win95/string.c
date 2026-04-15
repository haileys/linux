/* SPDX-License-Identifier: LGPL-2.1 OR MIT */
/*
 * string function definitions for NOLIBC
 * Copyright (C) 2017-2021 Willy Tarreau <w@1wt.eu>
 */

#include <linux/types.h>
#include <linux/string.h>
#include <wsl9x.h>

/* might be ignored by the compiler without -ffreestanding, then found as
 * missing.
 */

__attribute__((weak,unused,section(".text.nolibc_memmove")))
void *memmove(void *dst, const void *src, size_t len)
{
	size_t dir, pos;

	pos = len;
	dir = -1;

	if (dst < src) {
		pos = -1;
		dir = 1;
	}

	while (len) {
		pos += dir;
		((char *)dst)[pos] = ((const char *)src)[pos];
		len--;
	}
	return dst;
}

/* must be exported, as it's used by libgcc on ARM */
void *memcpy(void *dst, const void *src, size_t len);
__attribute__((weak,unused,section(".text.nolibc_memcpy")))
void *memcpy(void *dst, const void *src, size_t len)
{
	size_t pos = 0;

	while (pos < len) {
		((char *)dst)[pos] = ((const char *)src)[pos];
		pos++;
	}
	return dst;
}

void *memset(void *dst, int b, size_t len);
__attribute__((weak,unused,section(".text.nolibc_memset")))
void *memset(void *dst, int b, size_t len)
{
	char *p = dst;

	while (len--) {
		/* prevent gcc from recognizing memset() here */
		__asm__ volatile("");
		*(p++) = b;
	}
	return dst;
}

/* this function is only used with arguments that are not constants or when
 * it's not known because optimizations are disabled. Note that gcc 12
 * recognizes an strlen() pattern and replaces it with a jump to strlen(),
 * thus itself, hence the asm() statement below that's meant to disable this
 * confusing practice.
 */
size_t strlen(const char *str);
__attribute__((weak,unused,section(".text.nolibc_strlen")))
size_t strlen(const char *str)
{
	size_t len;

	for (len = 0; str[len]; len++)
		__asm__("");
	return len;
}

char *strstr(const char *haystack, const char *needle);
char *strstr(const char *haystack, const char *needle)
{
	size_t len_haystack, len_needle;

	len_needle = strlen(needle);
	if (!len_needle)
		return NULL;

	len_haystack = strlen(haystack);
	while (len_haystack >= len_needle) {
		if (!memcmp(haystack, needle, len_needle))
			return (char *)haystack;
		haystack++;
		len_haystack--;
	}

	return NULL;
}

void WSL9x_Printks(const char* str)
{
	WSL9x_Printk(str, strlen(str));
}
