/*! \file
 *  \brief \ref Watch device using the \ref LAPIC::Timer
 */

#pragma once

#include "types.h"
#include "interrupt/gate.h"

/*! \brief The \ref Watch device deals with timer interrupts.
 *
 * Handles \ref LAPIC::Timer interrupts, therefore managing the time slices and
 * triggering a \ref Scheduler::resume "thread switch" if necessary.
 */
class Watch : public Gate {
	// Prevent copies and assignments
	Watch(const Watch&)            = delete;
	Watch& operator=(const Watch&) = delete;

 private:
	static uint32_t ival;
	static uint8_t divide;
	static uint32_t counter;

 public:
	Watch() {}

	/*! \brief Windup / initialize
	 *
	 * Assigns itself to the \ref Plugbox and initializes the \ref LAPIC::Timer
	 * in such a way that regular interrupts are triggered approx. every `us`
	 * microseconds when \ref Watch::activate() is called.
	 * For this purpose, a suitable timer divisor is determined
	 * based on the timer frequency determined with \ref LAPIC::Timer::ticks().
	 * This timer divisor has to be as small as possible, but large enough to
	 * prevent the 32bit counter from overflowing.
	 *
	 * \param us Desired interrupt interval in microseconds.
	 * \return Indicates if the interval could be set.
	 *
	 * \todo Implement Method
	 */
	bool windup(uint32_t us);

	/*! \brief Prologue of timer interrupts
	 *
	 * \return `true` if the \ref Watch::epilogue should be executed.
	 *
	 *  \todo Implement Method
	 */
	bool prologue();

	/*! \brief Epilogue of timer interrupts
	 *
	 * Triggers the \ref Scheduler::resume "thread switch".
	 *
	 *  \todo Implement Method
	 */
	void epilogue();

	/*! \brief Retrieve the interrupt interval
	 *
	 *  \return Interval in microseconds
	 *
	 *  \todo Implement method
	 */
	uint32_t interval() const;

	/*! \brief Activate the timer on this core.
	 *
	 * The core local timer starts with the interval previously configured in
	 * \ref windup(). To get timer interrupts on all cores, this method must be
	 * called once per core (however, it is sufficient to call \ref windup only
	 * once since the APIC-Bus frequency is the same on each core).
	 *
	 *  \todo Implement method
	 */
	void activate() const;

};

extern Watch watch[];
