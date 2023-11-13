/*! \file
 *  \brief Contains the class Spinlock
 */

#pragma once

#include "machine/core.h"
#include "machine/cache.h"

/*! \brief By the use of Spinlocks, it is possible to serialize blocks of code
*   that might run parallel on multiple CPU cores.
 *
 *  \ingroup sync
 *
 *  Synchronization is implemented using a lock variable. Once a thread enters
 *  the critical area, it sets the lock variable (to a non-zero value); when
 *  this thread leaves the critical area, it resets the lock variable to zero.
 *  When trying to enter an already locked critical area, the trying thread
 *  actively waits until the critical area is free again.
 *
 *  Use the following two GCC intrinsics
 *    - `bool __atomic_test_and_set(void *ptr, int memorder)`
 *    - `void __atomic_clear (bool *ptr, int memorder)`
 *
 *  These intrinsics are translated into atomic, architecture-specific
 *  CPU instructions.
 *
 *  \note If you want that things just work, choose `__ATOMIC_SEQ_CST` as memorder.
 *        This is not the most efficient memory order but works reasonably well.
 *
 *  <a href="https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html">Atomic Builtins in GCC manual</a>
 */
class Spinlock {
	// Prevent copies and assignments
	Spinlock(const Spinlock& copy) = delete;
	Spinlock& operator=(const Spinlock&) = delete;

 public:
	/*! \brief Constructor; Initializes as unlocked.
	 *
	 *  \todo Complete Constructor (for \OOStuBS, or use \ref Ticketlock)
	 *
	 */
	Spinlock() {}

	/*! \brief Enters the critical area. In case the area is already locked,
	 *  \ref lock() will actively wait for the area can be entered.
	 *
	 *  \see \ref Core::pause()
	 *  \todo Implement Method (for \OOStuBS, or use \ref Ticketlock)
	 */
	void lock() {
	}
	/*! \brief Unblocks the critical area.
	 *
	 *
	 *  \todo Implement Method (for \OOStuBS, or use \ref Ticketlock)
	 */
	void unlock() {
	}
};

