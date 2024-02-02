/*! \file
 *  \brief \ref Bellringer that manages and activates time-triggered activities.
 */

#pragma once

#include "sync/bell.h"
#include "object/queue.h"

/*! \brief Manages and activates time-triggered activities.
 *  \ingroup ipc
 *
 *  The Bellringer is regularly activated and checks whether any of the bells should ring.
 *  The bells are stored in a Queue<Bell> that is managed by the Bellringer.
 *  A clever implementation avoids iterating through the whole list for every iteration by
 *  keeping the bells sorted and storing delta times. This approach leads to a complexity
 *  of O(1) for the method called by the timer interrupt in case no bells need to be rung.
 */
class Bellringer {
	// Prevent copies and assignments
	Bellringer();
	Bellringer(const Bellringer&)            = delete;
	Bellringer& operator=(const Bellringer&) = delete;

 private:
	static Queue<Bell> queue;

 public:
	/*! \brief Checks whether there are bells to be rung.
	 *
	 *  Every call to check elapses a tick. Once such a tick reduces a bells
	 *  remaining time to zero, the bell will be rung.
	 *
	 *  \todo Implement Method
	 */
	static void check();

	/*! \brief Passes a `bell` to the bellringer to be rung after `ms`
	 *  milliseconds.
	 *  \param bell Bell that should be rung after `ms` milliseconds
	 *  \param ms number of milliseconds that should be waited before
	 *  ringing the bell
	 *
	 *  \todo Implement Method
	 */
	static void job(Bell *bell, unsigned int ms);

	/*! \brief Cancel ticking & ringing a bell
	 *  \param bell Bell that should not be rung.
	 *
	 *  \todo Implement Method
	 */
	static void cancel(Bell *bell);

	/*! \brief Checks whether there are enqueued bells.
	 *  \return true if there are enqueued bells, false otherwise
	 *
	 *  \todo Implement Method
	 */
	static bool bellPending();
};
