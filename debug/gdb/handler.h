/*! \file
 *  \brief for \ref GDB_Stub \ref gdb_interrupt_handler "Interrupt handler" and its \ref DebugContext "context"
 */

#pragma once

#include "types.h"
#include "debug/assert.h"

/*! \brief Debug context (stored on stack during GDB interrupt)
 *
 * \see debug_handler
 * \see InterruptContext
 */
struct DebugContext {
	uint64_t gs : 16;
	uint64_t : 0;
	uint64_t fs : 16;
	uint64_t : 0;
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rbp;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t vector;
	uint64_t error_code : 32;
	uint64_t : 0;
	uint64_t rip;
	uint64_t cs : 16;
	uint64_t : 0;
	uint64_t eflags : 32;
	uint64_t : 0;
	uint64_t rsp;
	uint64_t ss : 16;
	uint64_t : 0;
} __attribute__((packed));

assert_size(DebugContext, 24 * 8);

/*! \brief High-Level interrupt handler for \ref GDB_Stub
 *
 * Similar to the default \ref interrupt_handler.
 * Called by the \ref gdb_interrupt_entry "entry function" written in assembly
 * (see `debug/gbd/handler.asm`) -- these routines are installed in the
 * \ref GDB_Stub() "constructor".
 *
 * After preparing the data this function calls GDB_Stub::handle, which
 * handles the communication with the host via the serial interface.
 *
 * \param context contains a pointer to the stack, which can be used to access
 *              the debug interrupt context.
 */
extern "C" void gdb_interrupt_handler(DebugContext * context);

/*!\brief Number of GDB interrupt entry function
 * (defined by `NUM_HANDLERS` in `gdb/handler.asm`)
 */
extern uint32_t gdb_interrupt_entries;

/*!\brief Array with pointers to the GDB interrupt entry functions
 * (`dbg_irq_entry_%VECTOR` in `gdb/handler.asm`)
 */
extern void * const gdb_interrupt_entry[];
