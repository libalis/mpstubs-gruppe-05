/*
 * Copyright (C) 2016  Matt Borgerson
 * Copyright (C) 2018  Bernhard Heinloth
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "debug/gdb/handler.h"
#include "debug/gdb/stub.h"
#include "debug/gdb/state.h"

#include "debug/output.h"

#include "interrupt/handler.h"
#include "machine/idt.h"
#include "machine/lapic.h"
#include "machine/core.h"

/*! \brief GDB instance
 *
 * (as it were singleton)
 */
static GDB_Stub * instance = 0;

/*! \brief store signal
 * to simplify debugging
 */
int GDB_Stub::signal = 0;

extern "C" void gdb_interrupt_handler(DebugContext *context) {
	// An active GDB instance is required
	if (instance == 0) {
		// Create interrupt context
		InterruptContext tmp;
		tmp.r11 = context->r11;
		tmp.r10 = context->r10;
		tmp.r9 = context->r9;
		tmp.r8 = context->r8;
		tmp.rdi = context->rdi;
		tmp.rsi = context->rsi;
		tmp.rdx = context->rdx;
		tmp.rcx = context->rcx;
		tmp.rax = context->rax;
		tmp.error_code = context->error_code;
		tmp.ip = context->rip;
		tmp.cs = context->cs;
		tmp.flags = context->eflags;
		tmp.sp = context->rsp;
		tmp.ss = context->ss;

		// Forward to default interrupt handler
		interrupt_handler(static_cast<Core::Interrupt::Vector>(context->vector), &tmp);

		// Abort
		return;
	}

	// Store state
	State::save(context);

	/* Since all cores on an SMP system share the same memory here, this can easily
	 * cause strange issues if several cores access it simultaneously.
	 * Panacea: Mutual exclusion!
	 *
	 * Of course, the combination of 'active waiting' and 'interrupt handler'
	 * should immediately cause bad stomach aches.
	 * The excuse: It is just for the debugger, which should be as simple as
	 * possible, used only during development and not in production systems.
	 *
	 * So better remember this as a simple hack only and not as best practice!
	 */
	static volatile unsigned slot = 0;
	static volatile unsigned wait = 0;

	// Has this core be interrupted by another core and therefore it is just supposed to wait?
	if (context->vector == Core::Interrupt::GDB) {
		__sync_synchronize();
		// Wait until the core handling GDB has finished
		while (wait > slot) {
			Core::pause();
		}
		__sync_synchronize();
	} else {
		// Mutual exclusion: Wait until the critical section is free
		unsigned tmp = __sync_fetch_and_add(&wait, 1);
		while (tmp > slot) {
			Core::pause();
		}

		// Stop all other cores by sending them a specific IPI
		LAPIC::IPI::sendOthers(Core::Interrupt::GDB);

		// Wait a few milliseconds
		for(int i = 0; i < 50000; ++i) {
			asm volatile ("cpuid\n\t" ::: "eax", "ebx", "ecx", "edx", "memory");
		}

		__sync_synchronize();

		// Store vector
		GDB_Stub::signal = context->vector;

		// Handle the context
		instance->handle();

		__sync_synchronize();

		// Release the critical section and all other cores
		slot = tmp + 1;
	}

	// Restore the (possibly modified) registers
	State::restore(context);

}

GDB_Stub::GDB_Stub(bool wait, bool debug_output, ComPort port, BaudRate baud_rate)
                 : Serial(port, baud_rate, DATA_8BIT, STOP_1BIT, PARITY_NONE), debug(debug_output) {
	// Some sort of singleton
	assert(instance == 0);

	// Assign instance variable
	instance = this;

	/* Install GDB debug interrupt handlers
	 *
	 * The entry code for the GDB interrupt handling of the traps is written in
	 * assembler in `debug/gdb/handler.asm`.
	 * These are similar to the `interrupt/handler.asm` entry functions, but
	 * explicitly save the complete state of the core (including the callee save
	 * and segment registers) and execute \ref gdb_interrupt_handler.
	 *
	 * Since the GDB stub can be activated dynamically (at runtime), the traps
	 * are initialized to use the \ref interrupt_handler "default interrupt handler"
	 * on startup.
	 * Only by calling this method the GDB interrupt handler is installed for
	 * the trap vectors.
	 *
	 * In this case, the \ref IDT "interrupt descriptor table (IDT)" have to be
	 * modified: The address in the descriptor originally points to
	 * `irq_entry_%VECTOR` (defined in the `interrupt/handler.asm`) and has
	 * to be changed to the address of the `debug/gdb/handler.asm` equivalent
	 * `interrupt_entry_%VECTOR`.
	 */
	for (int i = 0; i < 17; i++) {
		switch (i) {
			case 2:   // NMI
			case 15:  // reserved
				continue;

			default:
				IDT::handle(i, gdb_interrupt_entry[i]);
		}
	}

	// Handler to stop other cores during GDB interruption
	assert(Core::Interrupt::GDB < gdb_interrupt_entries);
	IDT::handle(Core::Interrupt::GDB, gdb_interrupt_entry[Core::Interrupt::GDB]);

	// Wait for connection
	if (wait) {
		breakpoint();
	}
}
