/*! \file
 *  \brief \ref GuardedBell, a \ref Guarded "guarded" interface for \ref Bell
 */

#pragma once

#include "sync/bell.h"

/*! \brief \ref Guarded interface to \ref Bell objects used by user applications.
 *
 * Implements the system call interface for class \ref Bell. All methods
 * provided by this class are wrappers for the respective method from the base
 * class, which provide additional synchronization by using the class \ref Guarded.
 */
class GuardedBell : private Bell {
	// Prevent copies and assignments
	GuardedBell(const GuardedBell&)            = delete;
	GuardedBell& operator=(const GuardedBell&) = delete;

 public:
	GuardedBell() {}

	/*! \copydoc Bell::sleep()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref Bell, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 *
	 *  \todo Implement method
	 */
	static void sleep(unsigned int ms) {
		(void) ms;
	}

};
