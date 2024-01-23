/*! \file
 *  \brief \ref IdleThread executed by the \ref Scheduler if no other \ref Thread is ready
 */

#pragma once

#include "thread/thread.h"

/*! \brief Thread that is executed when there is nothing to do for this core.
 *  \ingroup thread
 *
 * Using the IdleThread simplifies the idea of waiting and is an answer to the
 * questions that arise once the ready queue is empty.
 *
 * \note Instance of this class should *never* be inserted into the scheduler's
 *       ready queue, as the IdleThread should only be executed if there is no
 *       proper work to do.
  */
class IdleThread {
 public:

	/*! \brief Wait for a thread to become ready and sleep in the meantime.
	 *
	 *  \todo Implement Method
	 */
	void action();
};
