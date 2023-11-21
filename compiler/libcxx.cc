/*! \file
 *  \brief C++ runtime support functions
 */

#include "types.h"

void operator delete(void *ptr) {
	(void) ptr;
}

void operator delete(void *ptr, size_t size) {
	(void) ptr;
	(void) size;
}

extern "C" int __cxa_atexit(void (*func)(void *), void * arg, void * dso_handle) {
	// Registers a function that will be executed on exit.
	// We simply ignore those functions, as we don't need them for our operating systems.

	(void) func;
	(void) arg;
	(void) dso_handle;

	return 0;
}

extern "C" [[noreturn]] void __cxa_pure_virtual() {
	// Pure virtual function was called -- this is obviously not valid,
	// therefore we wait infinitely.
	while (true) {}
}
