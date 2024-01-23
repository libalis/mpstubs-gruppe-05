/*! \file
 *  \brief \ref GuardedSemaphore, a \ref Guarded "guarded" interface for \ref Semaphore
 */

#pragma once

#include "sync/semaphore.h"
#include "interrupt/guarded.h"

/*! \brief \ref Guarded interface to \ref Semaphore objects used by user applications.
 *
 * Implements the system call interface for class \ref Semaphore. All methods
 * provided by this class are wrappers for the respective method from the base
 * class, which provide additional synchronization by using the class \ref Guarded.
 */
class GuardedSemaphore : private Semaphore {
	// Prevent copies and assignments
	GuardedSemaphore(const GuardedSemaphore&)            = delete;
	GuardedSemaphore& operator=(const GuardedSemaphore&) = delete;

 public:
	/*! \brief The constructor passes the parameters to the base-class
	 *  constructor.
	 *
	 *
	 *  \todo Implement constructor
	 */
	explicit GuardedSemaphore(unsigned c) {}

	/*! \copydoc Semaphore::p()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref Semaphore, with the only difference that the call will be
	 *       protected by a \ref Guarded object.
	 *
	 *  \todo Implement method
	 */
	void p() {
	}

	/*! \copydoc Semaphore::v()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref Semaphore, with the only difference that the call will be
	 *       protected by a \ref Guarded object.
	 *
	 *  \todo Implement method
	 */
	void v() {
	}
};
