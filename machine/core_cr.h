/*! \file
 *  \brief Access to \ref Core::CR "Control Register" of a \ref Core "CPU core"
 */

#pragma once

#include "types.h"

namespace Core {
/*! \brief Control Register 0
 *
 * \see [ISDMv3, 2.5 Control Registers](intel_manual_vol3.pdf#page=74)
 */
enum CR0 : uintptr_t {
	CR0_PE = 1U << 0,   ///< Protected Mode enabled
	CR0_MP = 1U << 1,   ///< Monitor co-processor
	CR0_EM = 1U << 2,   ///< Emulation (no x87 floating-point unit present)
	CR0_TS = 1U << 3,   ///< Task switched
	CR0_ET = 1U << 4,   ///< Extension type
	CR0_NE = 1U << 5,   ///< Numeric error
	CR0_WP = 1U << 16,  ///< Write protect
	CR0_AM = 1U << 18,  ///< Alignment mask
	CR0_NW = 1U << 29,  ///< Not-write through caching
	CR0_CD = 1U << 30,  ///< Cache disable
	CR0_PG = 1U << 31,  ///< Paging
};

/*! \brief Control Register 4
 *
 * \see [ISDMv3, 2.5 Control Registers](intel_manual_vol3.pdf#page=77)
 */
enum CR4 : uintptr_t {
	CR4_VME        = 1U << 0,   ///< Virtual 8086 Mode Extensions
	CR4_PVI        = 1U << 1,   ///< Protected-mode Virtual Interrupts
	CR4_TSD        = 1U << 2,   ///< Time Stamp Disable
	CR4_DE         = 1U << 3,   ///< Debugging Extensions
	CR4_PSE        = 1U << 4,   ///< Page Size Extension
	CR4_PAE        = 1U << 5,   ///< Physical Address Extension
	CR4_MCE        = 1U << 6,   ///< Machine Check Exception
	CR4_PGE        = 1U << 7,   ///< Page Global Enabled
	CR4_PCE        = 1U << 8,   ///< Performance-Monitoring Counter enable
	CR4_OSFXSR     = 1U << 9,   ///< Operating system support for FXSAVE and FXRSTOR instructions
	CR4_OSXMMEXCPT = 1U << 10,  ///< Operating System Support for Unmasked SIMD Floating-Point Exceptions
	CR4_UMIP       = 1U << 11,  ///< User-Mode Instruction Prevention
	CR4_VMXE       = 1U << 13,  ///< Virtual Machine Extensions Enable
	CR4_SMXE       = 1U << 14,  ///< Safer Mode Extensions Enable
	CR4_FSGSBASE   = 1U << 16,  ///< Enables the instructions RDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE.
	CR4_PCIDE      = 1U << 17,  ///< PCID Enable
	CR4_OSXSAVE    = 1U << 18,  ///< XSAVE and Processor Extended States Enable
	CR4_SMEP       = 1U << 20,  ///< Supervisor Mode Execution Protection Enable
	CR4_SMAP       = 1U << 21,  ///< Supervisor Mode Access Prevention Enable
	CR4_PKE        = 1U << 22,  ///< Protection Key Enable
};

/*! \brief Access to the Control Register
 *
 * \see [ISDMv3, 2.5 Control Registers](intel_manual_vol3.pdf#page=73)
 * \tparam id Control Register to access
 */
template<uint8_t id>
class CR {
 public:
	/*! \brief Read the value of the current Control Register
	 *
	 * \return Value stored in the CR
	 */
	inline static uintptr_t read(void) {
		uintptr_t val;
		asm volatile("mov %%cr%c1, %0" : "=r"(val) : "n"(id) : "memory");
		return val;
	}

	/*! \brief Write a value into the current Control Register
	 *
	 * \param value Value to write into the CR
	 */
	inline static void write(uintptr_t value) {
		asm volatile("mov %0, %%cr%c1" : : "r"(value), "n"(id) : "memory");
	}
};
}  // namespace Core
