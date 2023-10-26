/*! \file
 *  \brief \ref LAPIC abstracts access to the Local \ref APIC
 */

#pragma once

#include "types.h"

/*! \brief Abstracts the local APIC (which is integrated into every CPU core)
 *  \ingroup interrupts
 *
 *  In modern (x86) PCs, every CPU core has its own Local APIC (LAPIC). The LAPIC is the link between the
 *  local CPU core and the I/O APIC (that takes care about external interrupt sources.
 *  Interrupt messages received by the LAPIC will be passed to the corresponding CPU core and trigger the
 *  interrupt handler on this core.
 *
 *  \see [ISDMv3 10.4 Local APIC](intel_manual_vol3.pdf#page=366)
 */
namespace LAPIC {
	/*! \brief Initialized the local APIC of the calling CPU core and sets the logical LAPIC ID in the LDR register
	 *  \param logical_id APIC ID to be set
	 */
	void init(uint8_t logical_id);

	/*! \brief Signalize EOI (End of interrupt)
	 *
	 *  Signalizes the LAPIC that the handling of the current interrupt finished. This function must be called at
	 *  the end of interrupt handling before ireting.
	 */
	void endOfInterrupt();

	/*! \brief Get the ID of the current core's LAPIC
	 *  \return LAPIC ID
	 */
	uint8_t getID();

	/*! \brief Get the Logical ID of the current core's LAPIC
	 *  \return Logical ID
	 */
	uint8_t getLogicalID();

	/*! \brief Set the Logical ID of the current core's LAPIC
	 *  \param id new Logical ID
	 */
	void setLogicalID(uint8_t id);

	/*! \brief Get version number of local APIC
	 *  \return version number
	 */
	uint8_t getVersion();

/*! \brief Inter-Processor Interrupts
 *
 *  For multi-core systems, the LAPIC enables sending messages (Inter-Processor Interrupts, IPIs) to
 *  other CPU cores and receiving those sent from other cores.
 *
 * \see [ISDMv3 10.6 Issuing Interprocessor Interrupts](intel_manual_vol3.pdf#page=380)
 */
namespace IPI {

	/*! \brief Check if the previously sent IPI has reached its destination.
	 *
	 *  \return `true` if the previous IPI was accepted from its target processor, otherwise `false`
	 */
	bool isDelivered();

	/*! \brief Send an Inter-Processor Interrupt (IPI)
	 * \param destination ID of the target processor (use APIC::getLAPICID(core) )
	 * \param vector      Interrupt vector number to be triggered
	 */
	void send(uint8_t destination, uint8_t vector);

	/*! \brief Send an Inter-Processor Interrupt (IPI) to a group of processors
	 * \param logical_destination Mask containing the logical APIC IDs of the target processors (use APIC::getLogicalLAPICID())
	 * \param vector              Interrupt vector number to be triggered
	 */
	void sendGroup(uint8_t logical_destination, uint8_t vector);

	/*! \brief Send an Inter-Processor Interrupt (IPI) to all processors (including self)
	 * \param vector Interrupt vector number to be triggered
	 */
	void sendAll(uint8_t vector);

	/*! \brief Send an Inter-Processor Interrupt (IPI) to all other processors (all but self)
	 * \param vector Interrupt vector number to be triggered
	 */
	void sendOthers(uint8_t vector);

	/*! \brief Send an INIT request IPI to all other processors
	 *
	 * \note Only required for startup
	 *
	 * \param assert if `true` send an INIT,
	 *               on `false` send an INIT Level De-assert
	 */
	void sendInit(bool assert = true);

	/*! \brief Send an Startup IPI to all other processors
	 *
	 * \note Only required for startup
	 *
	 * \param vector Pointer to a startup routine
	 */
	void sendStartup(uint8_t vector);

}  // namespace IPI

/*! \brief Local Timer (for each LAPIC / CPU)
 *
 * \see [ISDMv3 10.5.4 APIC Timer](intel_manual_vol3.pdf#page=378)
 */
namespace Timer {

	/*! \brief Determines the LAPIC timer frequency.
	 *
	 * This function will calculate the number of LAPIC-timer ticks passing in the course of one millisecond.
	 * To do so, this function will rely on PIT timer functionality and measure the tick delta between start
	 * and end of waiting for a predefined period.
	 *
	 * For measurement, the LAPIC-timer single-shot mode (without interrupts) is used; after measurement, the
	 * timer is disabled again.
	 *
	 * \note The timer is counting towards zero.
	 *
	 * \return Number of LAPIC-timer ticks per millisecond
	 */
	uint32_t ticks(void);

	/*! \brief Set the LAPIC timer.
	 *  \param counter  Initial counter value; decremented on every LAPIC timer tick
	 *  \param divide   Divider (power of 2, i.e., 1 2 4 8 16 32...) used as prescaler between bus frequency
	 *                  and LAPIC timer frequency: `LAPIC timer frequency = divide * bus frequency`.
	 *                  `divide` is a numerical parameter, the conversion to the corresponding bit mask is
	 *                  done internally by calling getClockDiv().
	 *  \param vector   Interrupt vector number to be triggered on counter expiry
	 *  \param periodic If set, the interrupt will be issued periodically
	 *  \param masked   If set, interrupts on counter expiry are suppressed
	 */
	void set(uint32_t counter, uint8_t divide, uint8_t vector, bool periodic, bool masked = false);

}  // namespace Timer
}  // namespace LAPIC
