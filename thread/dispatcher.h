/*! \file
 *  \brief \ref Dispatcher for \ref Thread threads
 */
#pragma once

#include "thread/thread.h"
#include "machine/core.h"

/*! \brief The dispatcher dispatches threads and, by that, puts the scheduler's
 *  decisions into action.
 *  \ingroup thread
 *
 *  The dispatcher manages the life pointer that refers to the currently
 *  active thread and performs the actual switching of processes.
 *  For single-core systems, a single life pointer is sufficient, as only a
 *  single thread can be active at any one time. On multi-core systems,
 *  every CPU core needs its own life pointer.
 */
class Dispatcher {
 private:
	static Thread* life_pointer[Core::MAX];

	/*! \brief private constructor to prevent instantiation
	 */
	Dispatcher();

	/*! \brief set the currently active thread
	 *  \param thread active Thread
	 */
	static void setActive(Thread* thread) {
		life_pointer[Core::getID()] = thread;
	}

 public:
	/*! \brief Returns the thread currently running on the CPU core calling
	 *  this method
	 *
	 *
	 *  \todo Implement Method
	 */
	static Thread* active() {
		return life_pointer[Core::getID()];
	}

	/*! \brief This method stores `first` as life pointer for this CPU core and
	 *  triggers the execution of `first` thread.
	 *  \note Only to be used for the first thread running on a CPU core.
	 *  \param first First thread to be executed on this CPU core.
	 *
	 *  \todo Implement Method
	 */
	static void go(Thread* first);

	/*! \brief Updates the life pointer to next and issues a thread change from
	 *  the old to the new life pointer.
	 *  \param next Next thread to be executed.
	 *
	 *  \todo Implement Method
	 */
	static void dispatch(Thread* next);
};
