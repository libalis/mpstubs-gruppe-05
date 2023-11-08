#include "assert.h"
#include "debug/output.h"

[[noreturn]] void assertion_failed(const char * exp, const char * func, const char * file, int line) {
	// TODO: Print error message (in debug window)
	DBG << exp << func << file << line << endl;
	// TODO: Then stop the current core permanently
	//       Use appropriate method from class Core to do so.
	Core::die();
}
