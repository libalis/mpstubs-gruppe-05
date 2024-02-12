#pragma once

#include "utils/string.h"

// All pointers passed to the VFS are (hopefully) accessed through these
// except for path strings (which are only ever read though).
static inline size_t copy_to_user(void *to, const void *from, size_t n) {
	memcpy(to, from, n);
	return n;
}

static inline size_t copy_from_user(void *to, const void *from, size_t n) {
	memcpy(to, from, n);
	return n;
}
