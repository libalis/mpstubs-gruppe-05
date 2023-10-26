#include "assert.h"

[[noreturn]] void assertion_failed(const char * exp, const char * func, const char * file, int line) {
	(void) exp;
	(void) func;
	(void) file;
	(void) line;
	// TODO: Print error message (in debug window)
	// TODO: Then stop the current core permanently
	//       Use appropriate method from class Core to do so.
	while(true) {}  // wait forever
}
