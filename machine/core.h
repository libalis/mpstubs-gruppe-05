/*! \file
 *  \brief Access to internals of a CPU \ref Core
 */

/*! \defgroup sync CPU Synchronization
 *
 * The synchronization module houses functions useful for orchestrating multiple processors and their activities.
 * Synchronisation, in this case, means handling the resource contention between multiple participants, running on
 * either the same or different cores.
 */

#pragma once

#include "types.h"
#include "machine/core_cr.h"
#include "machine/core_interrupt.h"
#include "machine/core_msr.h"

/*! \brief Implements an abstraction for CPU internals.
 *
 * These internals include functions to \ref Core::Interrupt "allow or deny interrupts",
 * access \ref Core::CR "control registers".
 */
namespace Core {

/*! \brief Maximum number of supported CPUs
 */
const unsigned MAX = 8;

/*! \brief Get the ID of the current CPU core
 * using \ref LAPIC::getID() with an internal lookup table.
 *
 * \return ID of current Core (a number between 0 and \ref Core::MAX)
 */
unsigned getID();

/*! \brief Initialize this CPU core
 *
 * Mark this core as *online* and setup the cores \ref LAPIC by assigning it a
 * unique \ref APIC::getLogicalAPICID() "logical APIC ID"
 *
 * \note Should only be called from \ref kernel_init() during startup.
 */
void init();

/*! \brief Deinitialize this CPU core
 *
 * Mark this Core as *offline*
 *
 * \note Should only be called from \ref kernel_init() after returning from `main()` or `main_ap()`.
 */
void exit();

/*! \brief Get number of available CPU cores
 *
 * \return total number of cores
 */
unsigned count();

/*! \brief Get number of successfully started (and currently active) CPU cores
 *
 * \return total number of online cores
 */
unsigned countOnline();

/*! \brief Check if CPU core is currently active
 * \param core_id ID of the CPU core
 * \return `true` if successfully started and is currently active
 */
bool isOnline(uint8_t core_id);

/*! \brief Gives the core a hint that it is executing a spinloop and should sleep "shortly"
 *
 * Improves the over-all performance when executing a spinloop by waiting a short moment reduce
 * the load on the memory.
 *
 * \see [ISDMv2, Chapter 4. PAUSE - Spin Loop Hint](intel_manual_vol2.pdf#page=887)
 */
inline void pause() {
	asm volatile("pause\n\t" : : : "memory");
}

/*! \brief Halt the CPU core until the next interrupt.
 *
 *  Halts the current CPU core such that it will wake up on the next interrupt. Internally, this function first enables
 *  the interrupts via `sti` and then halts the core using `hlt`. Halted cores can only be woken by interrupts.
 *  The effect of `sti` is delayed by one instruction, making the sequence `sti hlt` atomic (if interrupts were
 *  disabled previously).
 *
 * \see [ISDMv2, Chapter 4. STI - Set Interrupt Flag](intel_manual_vol2.pdf#page=1297)
 * \see [ISDMv2, Chapter 3. HLT - Halt](intel_manual_vol2.pdf#page=539)
 */
inline void idle() {
	asm volatile("sti\n\t hlt\n\t" : : : "memory");
}

/*! \brief Permanently halts the core.
 *
 *  Permanently halts the current CPU core. Internally, this function first disables the interrupts via `cli` and
 *  then halts the CPU core using `hlt`. As halted CPU cores can only be woken by interrupts, it is guaranteed that
 *  this core will be halted until the next reboot. The execution of die never returns.
 *  On multicore systems, only the executing CPU core will be halted permanently, other cores will continue execution.
 *
 * \see [ISDMv2, Chapter 3. CLI - Clear Interrupt Flag](intel_manual_vol2.pdf#page=245)
 * \see [ISDMv2, Chapter 3. HLT - Halt](intel_manual_vol2.pdf#page=539)
 */
[[noreturn]] inline void die() {
	while (true) {
		asm volatile("cli\n\t hlt\n\t" : : : "memory");
	}
}
}  // namespace Core
