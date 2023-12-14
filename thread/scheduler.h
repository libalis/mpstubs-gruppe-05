#pragma once

/*! \file
 *
 *  \brief \ref Scheduler to manage the \ref Thread "threads"
 */

#include "thread/dispatcher.h"
#include "thread/thread.h"
#include "object/queue.h"

/*! \brief The scheduler plans the threads' execution order and, from this,
 *  selects the next thread to be running.
 *  \ingroup thread
 *
 *  The scheduler manages the ready queue (a private \ref Queue object),
 *  that is the list of threads that are ready to execute. The scheduler
 *  arranges threads in a FIFO order, that is, when a thread is set ready, it
 *  will be appended to the end of the queue, while threads to be executed are
 *  taken from the front of the queue.
 */
class Scheduler : public Dispatcher {
	/*! \brief private constructor to prevent instantiation
	 */
	Scheduler();

 public:
	/*! \brief Start scheduling
	 *
	 *  This method starts the scheduling by removing the first thread from
	 *  the ready queue and activating it. \MPStuBS needs to call this method
	 *  once for every CPU core to dispatch the first thread.
	 *
	 *  \todo Implement Method
	 *
	 */
	static void schedule();

	/*! \brief Include a thread in scheduling decisions.
	 *
	 *  This method will register a thread for scheduling. It will be appended
	 *  to the ready queue and dispatched once its time has come.
	 *  \param that \ref Thread to be scheduled
	 *
	 *  \todo Implement Method
	 *
	 */
	static void ready(Thread *that);

	/*! \brief (Self-)termination of the calling thread.
	 *
	 *  This method can be used by a thread to exit itself. The calling
	 *  thread will not be appended to the ready queue; a reschedule will be
	 *  issued.
	 *
	 *  \todo Implement Method
	 *
	 */
	static void exit();

	/*! \brief Kills the passed thread
	 *
	 *  This method is used to kill the \ref Thread `that`.
	 *  For \OOStuBS, it is sufficient to remove `that` from the ready queue
	 *  and, thereby, exclude the thread from scheduling.
	 *  For \MPStuBS, a simple removal is not sufficient, as the thread might
	 *  currently be running on another CPU core. In this case, the thread needs
	 *  to be marked as *dying* (a flag checked by resume prior to enqueuing
	 *  into the ready queue)
	 *
	 *  \note The thread should be able to kill itself.
	 *
	 *  \todo Implement Method
	 */
	static void kill(Thread *that);

	/*! \brief Issue a thread change
	 *
	 *  This method issues the change of the currently active thread without
	 *  requiring the calling thread to be aware of the other threads.
	 *  Scheduling decisions, i.e. which thread will be run next, are made by
	 *  the scheduler itself with the knowledge of the currently ready threads.
	 *  The currently active thread is appended to the end of the queue; the
	 *  first thread in the queue will be activated (to implement the FIFO policy).
	 *
	 *  \todo Implement Method
	 *
	 */
	static void resume();

};
