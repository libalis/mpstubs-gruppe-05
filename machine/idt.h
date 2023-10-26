/*! \file
 *  \brief \ref IDT "Interrupt Descriptor Table (IDT)" containing the entry points for interrupt handling.
 */

#pragma once

#include "types.h"

/*! \brief "Interrupt Descriptor Table (IDT)
 * \ingroup interrupt
 *
 * \see [ISDMv3 6.14 Exception and Interrupt Handling in 64-bit Mode](intel_manual_vol3.pdf#page=200)
 */

namespace IDT {
	/*! \brief Gate types
	 *
	 * \see [ISDMv3 3.5 System Descriptor Types](intel_manual_vol3.pdf#page=99)
	 */
	enum Gate {
		GATE_TASK = 0x5,  ///< Task Gate
		GATE_INT  = 0x6,  ///< Interrupt Gate
		GATE_TRAP = 0x7,  ///< Trap Gate
	};

	/*! \brief Segment type
	 *
	 * \see [ISDMv3 3.5 System Descriptor Types](intel_manual_vol3.pdf#page=99)
	 */
	enum GateSize {
		GATE_SIZE_16 = 0,  ///< 16 bit
		GATE_SIZE_32 = 1,  ///< 32 / 64 bit
	};

	/*! \brief Descriptor Privilege Level
	 */
	enum DPL {
		DPL_KERNEL = 0,  ///< Ring 0 / Kernel mode
		/* DPLs 1 and 2 are unused */
		DPL_USER = 3,    ///< Ring 3 / User mode
	};

	/*! \brief Load the IDT's address and size into the IDT-Register via `idtr`.
	 */
	void load();

	/*! \brief Configure entry point for interrupt handling
	 *
	 *  The provided entry function ("handler") is required to, as first step, save the registers.
	 *
	 *  \param vector  Interrupt vector number for which the handler is to be set/changed
	 *  \param handler Low-level entry point for interrupt handling
	 *  \param type    Gate type (Interrupt, Trap, or Task)
	 *  \param size    16- or 32-bit
	 *  \param dpl     Permissions required for enter this interrupt handler (kernel- or user space)
	 *  \param present Denotes whether the IDT descriptor is marked as available
	 */
	void handle(uint8_t vector, void * handler, enum Gate type = Gate::GATE_INT,
	            enum GateSize size = GateSize::GATE_SIZE_32, enum DPL dpl = DPL::DPL_KERNEL, bool present = true);
}  // namespace IDT
