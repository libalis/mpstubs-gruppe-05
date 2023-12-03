/*! \file
 *  \brief Contains the class Ticketlock
 */

#pragma once

#include "machine/core.h"
#include "machine/cache.h"

/*! \brief By the use of Ticketlocks, it is possible to serialize blocks of code
 *  that might run parallel on multiple CPU cores.
 *
 *  \ingroup sync
 *
 *  Synchronization is implemented using a lock and a ticket variable.
 *  Once a thread tries to enter the critical area, it obtains a ticket by
 *  atomic increment of the ticket variable and waits until the lock variable
 *  is equal to its ticket.
 *  When a thread leaves the critical area, it increments the lock variable by
 *  one and thereby allows the next thread to enter the critical area.
 *
 *  \note If you want that things just work, choose `__ATOMIC_SEQ_CST` as memorder.
 *        This is not the most efficient memory order but works reasonably well.
 *
 *  <a href="https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html">Atomic Builtins in GCC manual</a>
 */
class Ticketlock {
	// Prevent copies and assignments
	Ticketlock(const Ticketlock& copy) = delete;
	Ticketlock& operator=(const Ticketlock&) = delete;

 private:
	volatile uint64_t ticket_current;
	volatile uint64_t ticket_count;

 public:
	volatile bool locked;

	/*! \brief Constructor
	 *
	 * \todo Complete Constructor (for \MPStuBS)
	 */
	Ticketlock() : ticket_current(0), ticket_count(0), locked(false) {}

	/*! \brief Enters the critical area. In case the area is already locked,
	 *  \ref lock() will actively wait for the area can be entered.
	 *
	 * \see \ref Core::pause()
	 * \todo Implement Method (for \MPStuBS)
	 */
	void lock();

	/*! \brief Unblocks the critical area.
	 *
	 * \todo Implement Method (for \MPStuBS)
	 */
	void unlock();
};

extern Ticketlock ticketlock;

extern Ticketlock corelock[Core::MAX];
extern Ticketlock BKL;
