
#ifndef mi_STRING_H
#define mi_STRING_H

#include <errno.h>
#include <string.h>

/* strnlen is not standard, strlen_s is C11... */
/* keep in mind there still is a buffer read overflow possible */
static size_t mi_strnlen_s(const char *src, size_t max_len)
{
	size_t len;
	
	if (src == NULL) {
		return 0U;
	}
	for (len = 0U; (*src != '\0') && (len < max_len); src++, len++);
	return len;
}

/* should be equivalent to C11 function except for the handler */
/* keep in mind there still is a buffer read overflow possible */
static int mi_strcpy_s(char* dst, size_t dst_size, const char* src)
{
	size_t src_len = 0U;
	if ((dst == NULL) || (dst_size == 0U)) {
		return EINVAL;
	}
	if (src == NULL) {
		dst[0] = '\0';
		return EINVAL;
	}
	src_len = mi_strnlen_s(src, dst_size);
	if (src_len >= dst_size) {
		return ERANGE;
	}
	memcpy(dst, src, src_len);
	dst[src_len] = '\0';
	return 0;
}

#endif /* mi_STRING_H */
