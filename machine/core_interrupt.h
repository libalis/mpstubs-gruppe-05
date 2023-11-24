/*! \file
 *  \brief \ref Core::Interrupt "Interrupt control" and \ref Core::Interrupt::Vector "interrupt vector list"
 */

#pragma once

#include "types.h"

namespace Core {
/*! \brief Exception and Interrupt control
 *
 * \see [ISDMv3, Chapter 6 Interrupt and Exception Handling](intel_manual_vol3.pdf#page=185)
 */
namespace Interrupt {

/*! \brief Bit in `FLAGS` register corresponding to the current interrupt state
 */
const uintptr_t FLAG_ENABLE = 1 << 9;

/*! \brief List of used interrupt vectors.
 *
 *  The exception vectors from `0` to `31` are reserved for traps, faults and aborts.
 *  Their behavior is different for each exception, some push an *error code*,
 *  some are not recoverable.
 *
 *  The vectors from `32` to `255` are user defined interrupts.
 *
 *  \see [ISDMv3, 6.15 Exception and Interrupt Reference](intel_manual_vol3.pdf#page=203)
 */
enum Vector {
	// Predefined Exceptions
	DIVISON_BY_ZERO          =  0,  ///< Divide-by-zero Error (at a `DIV`/`IDIV` instruction)
	DEBUG                    =  1,  ///< Debug exception
	NON_MASKABLE_INTERRUPT   =  2,  ///< Non Maskable Interrupt
	BREAKPOINT               =  3,  ///< Breakpoint exception (used for debugging)
	OVERFLOW                 =  4,  ///< Overflow exception (at `INTO` instruction)
	BOUND_RANGE_EXCEEDED     =  5,  ///< Bound Range Exceeded (at `BOUND` instruction)
	INVALID_OPCODE           =  6,  ///< Opcode at Instruction Pointer is invalid (you probably shouldn't be here)
	DEVICE_NOT_AVAILABLE     =  7,  ///< FPU "FPU/MMX/SSE" instruction but corresponding extension not activate
	DOUBLE_FAULT             =  8,  ///< Exception occurred while trying to call exception/interrupt handler
	// Coprocessor Segment Overrun (Legacy)
	INVALID_TSS              = 10,  ///< Invalid Task State Segment selector (see error code for index)
	SEGMENT_NOT_PRESENT      = 11,  ///< Segment not available (see error code for selector index)
	STACK_SEGMENT_FAULT      = 12,  ///< Stack segment not available or invalid (see error code for selector index)
	GENERAL_PROTECTION_FAULT = 13,  ///< Operation not allowed (see error code for selector index)
	PAGE_FAULT               = 14,  ///< Operation on Page (r/w/x) not allowed for current privilege (error code + `cr2`)
	// reserved (15)
	FLOATING_POINT_EXCEPTION = 16,  ///< x87 FPU error (at `WAIT`/`FWAIT`), accidentally \ref Core::CR0_NE set?
	ALIGNMENT_CHECK          = 17,  ///< Unaligned memory access in userspace (Exception activated by \ref Core::CR0_AM)
	MACHINE_CHECK            = 18,  ///< Model specific exception
	SIMD_FP_EXCEPTION        = 19,  ///< SSE/MMX error (if \ref Core::CR4_OSXMMEXCPT activated)
	// reserved (20 - 31)
	EXCEPTIONS               = 32,    ///< Number of exceptions

	// Interrupts
	TIMER                    = 32,    ///< Periodic CPU local \ref LAPIC::Timer interrupt
	KEYBOARD                 = 33,    ///< Keyboard interrupt (key press / release)
	PANIC                    = 34,    ///< Default handler for (unconfigured) interrupt events
	GDB                      = 35,    ///< Inter-processor interrupt to stop other CPUs for debugging in \ref GDB
	ASSASSIN                 = 100,   ///< Inter-processor interrupt to immediately stop threads running on other CPUs
	WAKEUP                   = 101,   ///< Inter-processor interrupt to WakeUp sleeping CPUs

	VECTORS                  = 256    ///< Number of interrupt vectors
};

/*! \brief Check if interrupts are enabled on this CPU
 *
 * This is done by pushing the `FLAGS` register onto stack,
 * reading it into a register and checking the corresponding bit.
 *
 *  \return `true` if enabled, `false` if disabled
 */
inline bool isEnabled() {
	uintptr_t out;
	asm volatile (
		"pushf\n\t"
		"pop %0\n\t"
		: "=r"(out)
		:
		: "memory"
	);
	return (out & FLAG_ENABLE) != 0;
}

/*! \brief Allow interrupts
 *
 *  Enables interrupt handling by executing the instruction `sti`.
 *  Since this instruction is delayed by one cycle, an subsequent `nop` is executed
 *  (to ensure deterministic behavior, independent from the compiler generated code)
 *
 *  A pending interrupt (i.e., those arriving while interrupts were disabled) will
 *  be delivered after re-enabling interrupts.
 *
 * \see [ISDMv2, Chapter 4. STI - Set Interrupt Flag](intel_manual_vol2.pdf#page=1297)
 */
inline void enable() {
	asm volatile("sti\n\t nop\n\t" : : : "memory");
}

/*! \brief Forbid interrupts
 *
 *  Prevents interrupt handling by executing the instruction `cli`.
 *  Will return the previous interrupt state.
 *  \return `true` if interrupts were enabled at the time of executing this function,
 *          `false` if they were already disabled.
 *
 *  \see [ISDMv2, Chapter 3. CLI - Clear Interrupt Flag](intel_manual_vol2.pdf#page=245)
 */
inline bool disable() {
	bool enabled = isEnabled();
	asm volatile ("cli\n\t" : : : "memory");

	return enabled;
}

/*! \brief Restore interrupt
 *
 *  Restore the interrupt state to the state prior to calling \ref disable() by using its return value.
 *
 *  \note This function will never disable interrupts, even if val is false!
 *        This function is designed to allow nested disabling and restoring of the interrupt state.
 *
 *  \param val if set to `true`, interrupts will be enabled; nothing will happen on false.
 */
inline void restore(bool val) {
	if (val) {
		enable();
	}
}
}  // namespace Interrupt
}  // namespace Core
