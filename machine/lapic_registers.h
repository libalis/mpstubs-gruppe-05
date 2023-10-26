/*! \file
 *  \brief Structures and macros for accessing \ref LAPIC "the local APIC".
 */

#pragma once

#include "types.h"

namespace LAPIC {
	// Memory Mapped Base Address
	extern volatile uintptr_t base_address;

	typedef uint32_t Register;

	/*! \brief Register Offset Index
	 *
	 * \see [ISDMv3 10.4.1 The Local APIC Block Diagram](intel_manual_vol3.pdf#page=368)
	 */
	enum Index : uint16_t {
		IDENTIFICATION                  = 0x020,  ///< Local APIC ID Register, RO (sometimes R/W). Do not change!
		VERSION                         = 0x030,  ///< Local APIC Version Register, RO
		TASK_PRIORITY                   = 0x080,  ///< Task Priority Register, R/W
		EOI                             = 0x0b0,  ///< EOI Register, WO
		LOGICAL_DESTINATION             = 0x0d0,  ///< Logical Destination Register, R/W
		DESTINATION_FORMAT              = 0x0e0,  ///< Destination Format Register, bits 0-27 RO, bits 28-31 R/W
		SPURIOUS_INTERRUPT_VECTOR       = 0x0f0,  ///< Spurious Interrupt Vector Register, bits 0-8 R/W, bits 9-1 R/W
		INTERRUPT_COMMAND_REGISTER_LOW  = 0x300,  ///< Interrupt Command Register 1, R/W
		INTERRUPT_COMMAND_REGISTER_HIGH = 0x310,  ///< Interrupt Command Register 2, R/W
		TIMER_CONTROL                   = 0x320,  ///< LAPIC timer control register, R/W
		TIMER_INITIAL_COUNTER           = 0x380,  ///< LAPIC timer initial counter register, R/W
		TIMER_CURRENT_COUNTER           = 0x390,  ///< LAPIC timer current counter register, RO
		TIMER_DIVIDE_CONFIGURATION      = 0x3e0   ///< LAPIC timer divide configuration register, RW
	};

	/*! \brief Get value from APIC register
	 *
	 * \param idx Register Offset Index
	 * \return current value of register
	 */
	Register read(Index idx);

	/*! \brief Write value to APIC register
	 *
	 * \param idx Register Offset Index
	 * \param value value to be written into register
	 */
	void write(Index idx, Register value);
}  // namespace LAPIC
