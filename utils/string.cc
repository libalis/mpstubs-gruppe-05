#include "string.h"
#include "types.h"

extern "C" char *strchrnul(const char *s, int c) {
	if (s != nullptr) {
		while(*s != '\0') {
			if (*s == c) {
				break;
			}
			s++;
		}
	}
	return const_cast<char *>(s);
}

extern "C" char *strchr(const char *s, int c) {
	if (s != nullptr) {
		s = strchrnul(s, c);
		if (*s == c) {
			return const_cast<char *>(s);
		}
	}
	return nullptr;
}

extern "C" int strcmp(const char *s1, const char *s2) {
	if (s1 == nullptr || s2 == nullptr) {
		return 0;
	}

	while(*s1 == *s2++) {
		if (*s1++ == '\0') {
			return 0;
		}
	}
	return static_cast<int>(*s1) - static_cast<int>(*(s2-1));
}

extern "C" int strncmp(const char *s1, const char *s2, size_t n) {
	if (s1 != nullptr && s2 != nullptr) {
		for (size_t i = 0; i < n; i++) {
			if (s1[i] != s2[i]) {
				return static_cast<int>(s1[i]) - static_cast<int>(s2[i]);
			} else if (s1[i] == '\0') {
				break;
			}
		}
	}
	return 0;
}

extern "C" size_t strlen(const char *s) {
	size_t len = 0;
	if (s != nullptr) {
		while (*s++ != '\0') {
			len++;
		}
	}

	return len;
}

extern "C" size_t strnlen(const char *s, size_t maxlen) {
	size_t len = 0;
	if (s != nullptr) {
		while (maxlen-- > 0 && *s++ != '\0') {
			len++;
		}
	}

	return len;
}

extern "C" char * strcpy(char *dest, const char *src) { //NOLINT
	char *r = dest;
	if (dest != nullptr && src != nullptr) {
		while ((*dest++ = *src++) != '\0') {}
	}
    return r;
}

extern "C" char * strncpy(char *dest, const char *src, size_t n) {
	char *r = dest;
	if (dest != nullptr && src != nullptr) {
		while (*src != '\0' && n-- != 0) {
			*dest++ = *src++;
		}
		while (n-- != 0) {
			*dest++ = '\0';
		}
	}
	return r;
}

extern "C" void* memcpy(void * __restrict__ dest, void const * __restrict__ src, size_t size) {
	uint8_t *destination = reinterpret_cast<uint8_t*>(dest);
	uint8_t const *source = (uint8_t const*)src;

	for(size_t i = 0; i != size; ++i) {
		destination[i] = source[i];
	}

	return dest;
}

extern "C" void* memmove(void * dest, void const * src, size_t size) {
	uint8_t *destination = reinterpret_cast<uint8_t*>(dest);
	uint8_t const *source = reinterpret_cast<uint8_t const*>(src);

	if(source > destination) {
		for(size_t i = 0; i != size; ++i) {
			destination[i] = source[i];
		}
	} else {
		for(size_t i = size; i != 0; --i) {
			destination[i-1] = source[i-1];
		}
	}

	return dest;
}

extern "C" void* memset(void *dest, int pattern, size_t size) {
	uint8_t *destination = reinterpret_cast<uint8_t*>(dest);

	for(size_t i = 0; i != size; ++i) {
		destination[i] = static_cast<uint8_t>(pattern);
	}

	return dest;
}

extern "C" int memcmp(const void * s1, const void * s2, size_t n) {
	const unsigned char * c1 = reinterpret_cast<const unsigned char*>(s1);
	const unsigned char * c2 = reinterpret_cast<const unsigned char*>(s2);

	for(size_t i = 0; i != n; ++i) {
		if (c1[i] != c2[i]) {
			return static_cast<int>(c1[i]) - static_cast<int>(c2[i]);
		}
	}

	return 0;
}
