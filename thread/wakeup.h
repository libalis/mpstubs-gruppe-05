#pragma once

/*! \file
 *  \brief \ref WakeUp allows to activate sleeping cores
 */

#include "interrupt/gate.h"
#include "interrupt/plugbox.h"

/*! \brief Interrupt handling used for waking sleeping cores.
 *
 *  In \MPStuBS, WakeUp IPIs are used to wakeup a sleeping core as soon as a new
 *  thread is ready to be scheduled/executed.
 *  The prologue for the WakeUp IPI explicitly should *NOT* request an epilogue.
 *
 *  Only required for \MPStuBS.
 */
class WakeUp : public Gate {
 public:
	/*! \brief Register interrupt handler.
	 *
	 *
	 *  \todo Implement Method (\MPStuBS)
	 */
	void activate();

	/*! \brief Interrupt is meant to only wakeup the core and, thus, signal
	 *  the availability of new threads in the ready queue.
	 *
	 *  \todo Implement Method (\MPStuBS)
	 */
	bool prologue();
};

extern WakeUp wakeup;
