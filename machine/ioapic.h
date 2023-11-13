/*! \file
 *  \brief \ref IOAPIC abstracts the access to the I/O \ref APIC
 */

#pragma once

#include "types.h"
#include "machine/ioapic_registers.h"
#include "machine/core_interrupt.h"

/*! \brief Abstraction of the I/O APIC that is used for management of external interrupts.
 *  \ingroup interrupts
 *
 *  The I/O APIC's Core component is the IO-redirection table. This table is used to configure a flexible mapping between
 *  the interrupt number and the external interruption. Entries within this table have a width of 64 bit.
 *  For convenience, the union \ref IOAPIC::RedirectionTableEntry should be used for modifying these tables (see
 *  file `ioapic_registers.h` for details).
 */

namespace IOAPIC {
	/*! \brief Initializes the I/O APIC.
	 *
	 *  This function will initialize the I/O APIC by initializing the IO-redirection table with sane default values.
	 *  The default interrupt-vector number is chosen such that, in case the interrupt is issued, the panic handler
	 *  is executed. In the beginning, all external interrupts are disabled within the I/O APIC.
	 *  Apart from the redirection table, the `APICID` (read from the system description tables during boot) needs to
	 *  be written to the `IOAPICID` register (see \ref APIC::getIOAPICID() )
	 *
	 *  \todo Implement Function
	 */
	void init();

	/*! \brief Creates a mapping between an interrupt vector and an external interrupt.
	 *
	 *  \param slot         Number of the slot (i.e., the external interrupt) to configure.
	 *  \param vector       Number of the interrupt vector that will be issued for the external interrupt.
	 *  \param trigger_mode Edge or level triggered interrupt signaling (level-triggered interrupts required for the
	                        optional serial interface)
	 *  \param polarity     Polarity of the interrupt signaling (active high or active low)
	 *
	 *  \todo Implement Function
	 */
	void config(uint8_t slot, Core::Interrupt::Vector vector, TriggerMode trigger_mode = TriggerMode::EDGE,
	            Polarity polarity = Polarity::HIGH);

	/*! \brief Enables the redirection of particular external interrupts to the CPU(s).
	 *
	 *  To fully enable interrupt handling, the interrupts must be enabled for every CPU (e.g., by calling
	 *  \ref Core::Interrupt::enable() in main)
	 *
	 * \param slot Number of the external interrupt that should be enabled.
	 *
	 *  \todo Implement Function
	*/
	void allow(uint8_t slot);

	/*! \brief Selectively masks external interrupts by slot number.
	 *  \param slot Slot number of the interrupt to be disabled.
	 *
	 *  \todo Implement Function
	 */
	void forbid(uint8_t slot);

	/*! \brief Check whether an external interrupt source is masked.
	 *  \param slot Slot number of the interrupt to be checked.
	 *  \return Returns `true` iff the interrupt is unmasked, `false` otherwise
	 *
	 *  \todo Implement Function
	 */
	bool status(uint8_t slot);
}  // namespace IOAPIC
