#include "compiler/libc.h"
#include "types.h"

/*! \brief Function pointer for initialization/finalization functions for global objects
 * required since GCC 4.7 and later.
 *
 * These symbols appear kind of magically due to the compiler
 */
extern void(*__preinit_array_start[]) ();
extern void(*__preinit_array_end[]) ();
extern void(*__init_array_start[]) ();
extern void(*__init_array_end[]) ();
extern void(*__fini_array_start[]) ();
extern void(*__fini_array_end[]) ();

extern "C" void _init();
extern "C" void _fini();

namespace CSU {

void initializer() {
	const unsigned int preinit_size = __preinit_array_end - __preinit_array_start;
	for (unsigned int i = 0; i != preinit_size; ++i) {
		(*__preinit_array_start[i])();
	}

	_init();

	const size_t size = __init_array_end - __init_array_start;
	for (size_t i = 0; i < size; i++) {
		(*__init_array_start[i])();
	}
}

void finalizer() {
	const unsigned int fini_size = __fini_array_end - __fini_array_start;
	for (unsigned int i = 0; i != fini_size; ++i) {
		(*__fini_array_start[i])();
	}

	_fini();
}

}  // namespace CSU
