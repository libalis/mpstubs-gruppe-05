/*! \file
 *  \brief Default interrupt handling device \ref Panic
 */

#pragma once

#include "interrupt/gate.h"

/*! \brief Default handler for (unconfigured) interrupt events
 *
 * \ref Panic is used to handle unconfigured interrupts and exceptions.
 * After a generic error message is displayed, the core is stopped permanently.
 *
 * During initialization of \ref Plugbox this fake device is assigned for
 * all \ref Core::Interrupt::Vector "interrupt vectors"
 */
class Panic : public Gate {
	// Prevent copies and assignments
	Panic(const Panic&)            = delete;
	Panic& operator=(const Panic&) = delete;

 public:
	/*! \brief Constructor
	 */
	Panic() {}

	/*! \brief Simplest possible interrupt handling: Displaying an error message
	* and stopping the current core.
	 *
	 *  \todo Implement Method
	 */
	void trigger() override;
};
