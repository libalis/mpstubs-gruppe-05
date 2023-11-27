/*! \file
 *  \brief \ref Guard synchronizes access to epilogue level
 */

#pragma once

#include "interrupt/gate.h"

/*! \brief Synchronizes the kernel with interrupts using the Prologue/Epilogue Model
 *  \ingroup interrupts
 *
 * The Guard is used to synchronize between "normal" core activities (currently
 * just the text output, later system calls) and interrupt handling routines.
 * For this purpose, \ref GateQueue provides \ref Guard with a queue for each
 * Core, in which gate objects can be added. This is necessary if the critical
 * section is occupied at the time an interrupt occurs, and the `epilogue()`
 * method cannot be executed immediately. The queued epilogues are processed
 * when leaving the critical section.
 *
 * **Hints:**
 *  <ul>
 *      <li>The epilogue queue is a central data structure, whose consistency
 *      must be ensured. The implementation provided by the \ref GateQueue
 *      may be not interrupt-transparent. Either you are able to implement a
 *      perfect interrupt-transparent queue yourself, or you simply disable
 *      interrupts during operations on the queue (hard synchronization).</li>
 *
 *      <li>In \MPStuBS, you need a separate epilogue queue for each core,
 *      in which each processor serializes *its* epilogues. However, epilogues
 *      on different cores could then be executed in parallel, since the
 *      critical section is managed separately on a per-core base. This must be
 *      prevented by using a global \ref Ticketlock to avoid concurrent
 *      execution of epilogues -- there must never be more than one epilogue
 *      active on the whole system at the same time!<br>
 *      *Please note:* This [giant lock](https://en.wikipedia.org/wiki/Giant_lock)
 *      (synchronizing all cores) should not be confused with the (core-specific)
 *      lock variable that marks only the entry to the epilogue level on the
 *      corresponding core!</li>
 *
 *      <li>\ref Gate objects must not be enqueued multiple times in the same
 *      queue. So if two interrupts of the same type occur so quick (for
 *      \MPStuBS: on the same core) that the corresponding epilogue has
 *      not yet been handled, you must not enqueue the same gate object again.
 *      The \ref GateQueue::enqueue() "enqueue" methods should prevent this.</li>
 *
 *      <li>Interrupts should be disabled for as short as possible. Due to this
 *      reason, the prologue/epilogue model allows epilogues to be interrupted
 *      by prologues. This means that interrupts should be
 *      \ref Core::Interrupt::enable "enabled" again before the epilogue is
 *      executed (this includes notifying the APIC about the
 *      \ref LAPIC::endOfInterrupt() "End-Of-Interrupt")
 *    </ul>
 */
namespace Guard {

	/*! \brief Entering the critical section from level 0.
	 *
	 * Entering the critical section has to be handled differently depending on
	 * the system: In a single-core system it is sufficient to mark the entry
	 * by just setting a lock variable (since only one control flow can enter
	 * the critical section at the same time). However, as soon as there are
	 * multiple cores, this is no longer the case. If a core wants to enter the
	 * critical section while *another* core is already in there, it should
	 * (actively) wait in this method until the critical area is released again.
	 *
	 *  \todo Implement Method
	 */
	void enter();

	/*! \brief Leaving the critical section.
	 *
	 * Leaves the critical section and processes all remaining (enqueued) epilogues.
	 *
	 *  \todo Implement Method
	 */
	void leave();

	/*! \brief A prologue wants its epilogue to be processed (entering from level 1).
	 *
	 * This method is called by \ref interrupt_handler if the previously executed
	 * \ref Gate::prologue has returned `true` -- indicating that it needs its
	 * epilogue to be executed as well.
	 * Whether this is done immediately or the epilogue just enqueued to the
	 * epilogue queue depends on whether the critical section on *this* Core is
	 * accessible or not.
	 *
	 *  \todo Implement Method
	 */
	void relay(Gate *item);
}  // namespace Guard
