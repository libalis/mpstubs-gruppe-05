#pragma once

/*! \file
 *  \brief Contains the class Waitingroom.
 */

/*! \defgroup ipc IPC Subsystem
 *
 * The IPC subsystem provides a synchronization interface for the thread
 * abstraction level. This synchronization includes synchronization between
 * threads (semaphores), as well as with their surroundings (bells).
 */

#include "object/queue.h"

class Thread;

/*! \brief List of threads waiting for an event.
 *  \ingroup ipc
 *
 *
 *  The class Waitingroom implements a list of threads that all wait for one
 *  particular event.
 *
 *  The destructor should be virtual to properly cleanup derived classes.
 */
class Waitingroom : public Queue<Thread> {
	// Prevent copies and assignments
	Waitingroom(const Waitingroom&)            = delete;
	Waitingroom& operator=(const Waitingroom&) = delete;

 public:
	/*! \brief Constructor
	 *
	 * Creates an empty Waitingroom
	 */
	Waitingroom() {}

	/*! \brief Destructor
	 *
	 *  The destructor removes and awakes all remaining threads.
	 *
	 *  \todo Implement Destructor
	 */
	virtual ~Waitingroom();

	/*! \brief Remove a given thread prematurely from the Waitingroom.
	 *
	 *
	 *  \todo Implement Method
	 */
	virtual void remove(Thread *customer);
};
