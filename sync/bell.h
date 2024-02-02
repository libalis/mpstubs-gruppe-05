#pragma once

/*! \file
 *  \brief \ref Bell, a synchronization object for sleeping
 */

#include "sync/waitingroom.h"
#include "object/queue.h"

class Bellringer;

/*! \brief Synchronization object allowing to sleep for given timespan
 *  \ingroup ipc
 *
 *  A bell is a synchronization object enabling one thread to sleep for a
 *  particular timespan.
 */
class Bell : public Waitingroom, public Queue<Bell>::Node {
	// Prevent copies and assignments
	Bell(const Bell&)            = delete;
	Bell& operator=(const Bell&) = delete;

 public:
	unsigned int ms;

	/*! \brief Constructor
	 *
	 *  Constructs a new bell; the newly created bell is, at first, disabled.
	 */
	explicit Bell(unsigned int ms) : ms(ms) {}

	/*! \brief Ring the bell
	 *
	 *  Method called by the Bellringer once the waiting time passed.
	 *  Wakes up the sleeping thread(s).
	 *
	 *  \todo Implement Method
	 */
	void ring();

	/*! \brief Creates a temporary bell object and sleeps for the given timespan
	 *  \param ms time in milliseconds
	 *  \todo Implement Method
	 */
	static void sleep(unsigned int ms);
};
