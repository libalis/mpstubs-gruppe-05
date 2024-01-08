/*! \file
 *  \brief \ref Assassin handles IPIs triggered by \ref Scheduler::kill
 */

#pragma once

#include "interrupt/gate.h"

/*! \brief Handling for the "killer"-IPI, that is a message indicating that a
 *  thread should be terminated.
 *  \ingroup thread
 *
 *
 *  Only required for \MPStuBS.
 *
 *  If one thread wants to terminate another thread by calling \ref Scheduler::kill(),
 *  and the thread to be killed is currently being executed on another CPU, this
 *  CPU needs to be notified that the active process is to be killed.
 *  \MPStuBS relies on an inter-processor interrupt (IPI) for this purpose.
 *  The class Assassin shall handle the IPI by removing the active thread from
 *  scheduling and issue a reschedule iff the active thread has a set kill flag.
 */
class Assassin : public Gate {
 public:
	/*! \brief Configure the Assassin
	 *
	 *  Setup the plugbox, such that the assassin will be executed once the
	 *  corresponding IPI is issued.
	 *
	 *  \todo Implement Method
	 *
	 */
	void hire();

	/*! \brief Interrupt Prologue
	 *
	 *  There is nothing to do in the prologue except requesting an epilogue.
	 *  \return `true` to request an epilogue
	 *
	 *  \todo Implement Method
	 *
	 */

	bool prologue();
	/*! \brief Interrupt Epilogue
	 *
	 *  The epilogue now should check the dying flag of the currently
	 *  running thread and, if set, issue a rescheduling.
	 *
	 *  \todo Implement Method
	 *
	 */
	virtual void epilogue();
};
