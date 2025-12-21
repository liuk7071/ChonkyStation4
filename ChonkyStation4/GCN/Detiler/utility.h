#ifndef _U_UTILITY_H_
#define _U_UTILITY_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define uasize(_array) (sizeof(_array) / sizeof(_array[0]))

#define umin(l, r) (l > r ? r : l)
#define umax(l, r) (l > r ? l : r)
#define uclamp(val, l, h) (val < l ? l : (val > h ? h : val))

static inline bool ispow2aligned(uint64_t value, uint64_t alignment) {
	return ((value & (alignment - 1)) == 0);
}

static inline _Noreturn void fatal(const char* msg) {
	puts(msg);
	exit(EXIT_FAILURE);
}

static inline _Noreturn void fatalf(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	putc('\n', stdout);

	exit(EXIT_FAILURE);
}

static inline char* hexstr(
    char* buf, size_t bufsize, const void* data, size_t datalen
) {
	static const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
				      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	const size_t requiredsize = datalen * 2 + 1;
	if (requiredsize > bufsize) {
		return NULL;
	}

	for (size_t i = 0; i < datalen; i += 1) {
		const uint8_t upper = (((const uint8_t*)data)[i] & 0xf0) >> 4;
		const uint8_t lower = ((const uint8_t*)data)[i] & 0xf;
		buf[i * 2] = hexmap[upper];
		buf[i * 2 + 1] = hexmap[lower];
	}
	buf[datalen * 2] = 0;

	return buf;
}

static inline char* ahexstr(const void* data, size_t datalen) {
	const size_t requiredsize = datalen * 2 + 1;
	char* str = malloc(requiredsize);
	return hexstr(str, requiredsize, data, datalen);
}

// openbsd's strlcpy
static inline size_t u_strlcpy(char* dst, const char* src, size_t dsize) {
	const char* osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0'; /* NUL-terminate dst */
		while (*src++)
			;
	}

	return (src - osrc - 1); /* count does not include NUL */
}

// taken from openbsd
static inline size_t u_strlcat(char* dst, const char* src, size_t dsize) {
	const char* odst = dst;
	const char* osrc = src;
	size_t n = dsize;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end. */
	while (n-- != 0 && *dst != '\0')
		dst++;
	dlen = dst - odst;
	n = dsize - dlen;

	if (n-- == 0)
		return (dlen + strlen(src));
	while (*src != '\0') {
		if (n != 0) {
			*dst++ = *src;
			n--;
		}
		src++;
	}
	*dst = '\0';

	return (dlen + (src - osrc)); /* count does not include NUL */
}

// taken from musl
static inline char* u_strtok_r(char* s, const char* sep, char** p) {
	if (!s && !(s = *p))
		return NULL;
	s += strspn(s, sep);
	if (!*s)
		return *p = 0;
	*p = s + strcspn(s, sep);
	if (**p)
		*(*p)++ = 0;
	else
		*p = 0;
	return s;
}

static inline uint32_t fui(float f) {
	union {
		float f;
		int32_t i;
		uint32_t ui;
	} fi;
	fi.f = f;
	return fi.ui;
}
static inline float uif(uint32_t u) {
	union {
		float f;
		int32_t i;
		uint32_t ui;
	} fi;
	fi.ui = u;
	return fi.f;
}

#endif	// _U_UTILITY_H_
