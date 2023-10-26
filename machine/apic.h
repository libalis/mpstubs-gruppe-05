/*! \file
 *  \brief Gather system information from the \ref ACPI about the \ref APIC "Advanced Programmable Interrupt Controller (APIC)"
 */

#pragma once

#include "types.h"

/*! \brief Information about the (extended) Advanced Programmable Interrupt Controller
 */
namespace APIC {
	/*! \brief Historic order of interrupt lines (PIC)
	 */
	enum Device {
		TIMER         = 0,   ///< Programmable Interrupt Timer (\ref PIT)
		KEYBOARD      = 1,   ///< Keyboard
		COM1          = 4,   ///< First serial interface
		COM2          = 3,   ///< Second serial interface
		COM3          = 4,   ///< Third serial interface (shared with COM1)
		COM4          = 3,   ///< Forth serial interface (shared with COM2)
		FLOPPY        = 6,   ///< Floppy device
		LPT1          = 7,   ///< Printer
		REALTIMECLOCK = 8,   ///< Real time clock
		PS2MOUSE      = 12,  ///< Mouse
		IDE1          = 14,  ///< First hard disk
		IDE2          = 15   ///< Second hard disk
	};

	/*! \brief Invalid APIC ID
	 *
	 * The highest address is reserved according to xAPIC specification
	 */
	const uint8_t INVALID_ID = 0xff;

	/*! \brief Executes system detection
	 *
	 * Searches and evaluates the APIC entries in the \ref ACPI table.
	 * This function recognizes a possibly existing multicore system.
	 * After successful detection, the number of available CPUs (which is equal
	 * to the number of  \ref LAPIC "local APICs") ) can be queried
	 * using the method \ref Core::count().
	 *
	 * \note Called by \ref kernel_init() on BSP
	 *
	 * \return `true` if detection of the APIC entries was successful
	 */
	bool init();

	/*! \brief Queries the physical I/O-APIC address determined during system boot
	 *
	 * \return Base address of the (first & only supported) I/O APIC
	 */
	uintptr_t getIOAPICAddress();

	/*! \brief Queries of ID of the I/O-APIC determined during system boot
	 *
	 * \return Identification of the (first & only supported) I/O APIC
	 */
	uint8_t getIOAPICID();

	/*! \brief Returns the pin number the \p device is connected to.
	 */
	uint8_t getIOAPICSlot(APIC::Device device);

	/*! \brief Returns the logical ID of the Local APIC passed for \a core.
	 *
	 *  The LAPIC's logical ID is set (by StuBS) during boot such that exactly one bit is set per CPU core.
	 *  For core 0, bit 0 is set in its ID, while core 1 has bit 1 set, etc.
	 *
	 *  \param core The queried CPU core
	 */
	uint8_t getLogicalAPICID(uint8_t core);

	/*! \brief Get the Local APIC ID of a CPU
	 *  \param core Query CPU core number
	 *  \return LAPIC ID of CPU or INVALID_ID if invalid CPU ID
	 */
	uint8_t getLAPICID(uint8_t core);

}  // namespace APIC
