/*! \file
 *  \brief \ref Thread abstraction required for multithreading
 */

/*! \defgroup thread Multithreading
 *  \brief The Multithreading Subsystem
 *
 * The group Multithreading contains all elements that form the foundation
 * of CPU multiplexing. This module's objective is to provide the abstraction
 * thread that provides a virtualised CPU for the user's applications.
 */

#pragma once

#include "machine/context.h"
#include "object/queue.h"

/*! \brief The is an object used by the scheduler.
 *  \ingroup thread
 */
class Thread : public Queue<Thread>::Node {
 public:
	/*! \brief Stack size for each thread
	 */
	static const size_t STACK_SIZE = 4096;

 protected:
	/*! \brief Current stack pointer of thread for context switch
	 */
	StackPointer stackpointer;

	/*! \brief Function to start a thread.
	 *
	 *  For the first activation of a thread, we need a "return address"
	 *  pointing to a function that will take care of calling C++ virtual
	 *  methods. For this purpose, we use this `kickoff()` function.
	 *
	 *  <b>Activating kickoff</b>
	 *
	 *  The thread initialization via \ref prepareContext() not only initializes
	 *  the Stack for the first thread change, but also pushes the address of
	 *  `kickoff()` as return address to the stack.
	 *  Consequently, the first execution of \ref context_switch() will start
	 *  execution by returning to the beginning of `kickoff()` .
	 *
	 *  This `kickoff()` function simply calls the \ref action() method on the
	 *  thread passed as parameter and, thus, resolves the virtual C++ method.
	 *
	 *  \note As this function is never actually called, but only executed by
	 *        returning from the threads's initial stack, it may never return.
	 *        Otherwise garbage values from the stack will be interpreted as
	 *        return address and the system might crash.
	 *
	 *  \param object Thread to be started
	 *
	 *  \todo Implement Method
	 */
	static void kickoff(Thread* object);

 public:
	/*! \brief Unique ID of thread
	 */
	const size_t id;

	/*! \brief Marker for a dying thread
	 */
	volatile bool kill_flag;

	/*! \brief Constructor
	 *  Initializes the context using \ref prepareContext with the highest
	 *  aligned address of the `reserved_stack_space` array as stack pointer
	 *  (top of stack).
	 * \note Remember: Stacks grow to the lower addresses on x86!
	 *
	 *  \todo Implement constructor
	 */
	Thread();

	/*! \brief Activates the first thread on this CPU.
	 *
	 *  Calling the method starts the first thread on the calling CPU.
	 *  From then on, \ref Thread::resume() must be used all subsequent context
	 *  switches.
	 *
	 *  \todo Implement Method
	 *
	 */
	void go();

	/*! \brief Switches from the currently running thread to the `next` one.
	 *
	 *  The values currently present in the non-scratch (callee-saved) registers
	 *  will be stored on this thread's stack; the corresponding values belonging
	 *  to `next` thread will be loaded (from `next`'s stack).
	 *  \param next Pointer to the next thread.
	 *
	 *  \todo Implement Method
	 *  \opt To detect stack overflows you can check if the bottom of the stack
	 *       still contains a predefined value (which was set in constructor).
	 *
	 */
	void resume(Thread *next);

	/*! \brief Method that contains the thread's program code.
	 *
	 *  Derived classes are meant to override this method to provide
	 *  meaningful code to be run in this thread.
	 */
	virtual void action() = 0;

};
