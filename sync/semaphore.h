#pragma once

/*! \file
 *  \brief \ref Semaphore for synchronization of threads.
 */

#include "sync/waitingroom.h"

/*! \brief Semaphore used for synchronization of threads.
 *  \ingroup ipc
 *
 *  The class Semaphore implements the concept of counting semaphores.
 *  The waiting list is provided by the base class Waitingroom.
 */
class Semaphore : public Waitingroom {
	// Prevent copies and assignments
	Semaphore(const Semaphore&)            = delete;
	Semaphore& operator=(const Semaphore&) = delete;

 public:
	/*! \brief Constructor; initialized the counter with provided value `c`
	 *  \param c Initial counter value
	 *
	 *  \todo Implement Constructor
	 */
	explicit Semaphore(unsigned c = 0) {
		(void)c;
	}

	/*! \brief Wait for access to the critical area.
	 *
	 *  Enter/Wait operation: If the counter is greater than 0, then it is
	 *  decremented by one. Otherwise the calling thread will be enqueued
	 *  into the Waitingroom and marked as blocked.
	 *
	 *  \todo Implement Method
	 */
	void p();

	/*! \brief Leave the critical area.
	 *
	 *  Leave operation: If there are threads in the Waitingroom, wake the
	 *  first one; otherwise increment the counter by one.
	 *
	 *  \todo Implement Method
	 */
	void v();
};
