/*! \file
 *  \brief \ref Plugbox allows assigning \ref Gate "devices" to \ref Core::Interrupt::Vector "interrupt vectors"
 */

#pragma once

#include "machine/core_interrupt.h"
#include "interrupt/gate.h"

/*! \brief Object-oriented abstraction of an device interrupt table
 *  \ingroup interrupts
 *
 * This allows you to specify the device handler for each hardware and software
 * interrupt and processor exception. Each device source is represented by a
 * \ref Gate object. These are located in an array with 256 elements, using
 * the index as the vector number.
 */
namespace Plugbox {
	/*! \brief Register a \ref Gate object to handle a specific interrupt.
	 *
	 *  \param vector Interrupt vector handled by the handler routine
	 *  \param gate Object with the handler routine
	 *
	 *  \todo Implement Method
	 */
	void assign(Core::Interrupt::Vector vector, Gate *gate);

	/*! \brief Query the \ref Gate object for a specific interrupt
	 *
	 *  \param vector Interrupt vector number
	 *  \return Reference to the registered \ref Gate object
	 *
	 *  \todo Implement Method
	 */
	Gate* report(Core::Interrupt::Vector vector);
};  // namespace Plugbox
