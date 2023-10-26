/*! \file
 *  \brief \ref interrupt_handler() Interrupt handler
 */
#pragma once

#include "types.h"
#include "machine/core_interrupt.h"

/*! \defgroup interrupts Interrupt Handling
 *  \brief The interrupt subsystem
 *
 * The interrupt subsystem of StubBS contains all functionality to accept
 * interrupts from the hardware and process them.
 * In later exercises the interrupts will enable applications to
 * execute core functionality (system calls).
 * The entry point for the interrupt subsystem is the function
 * 'interrupt_entry_VECTOR' (in `interrupt/handler.asm`).
 */

/*! \brief Preserved interrupt context
 *
 * After an interrupt was triggered, the core first saves the basic context
 * (current code- & stack segment, instruction & stack pointer and the status
 * flags register) and looks up the handling function for the vector using the
 * \ref IDT. In StuBS for each vector an own `interrupt_entry_VECTOR` function
 * (written in assembly in `interrupt/handler.asm`) was registered during boot
 * by \ref kernel_init(), which all save the scratch registers on the stack
 * before calling the C++ function \ref interrupt_handler().
 * The high-level handler gets a pointer to the part of the stack which
 * corresponds to the \ref InterruptContext structure as second parameter.
 * After returning from the high-level handler, the previous state is restored
 * from this context (scratch register in assembly and basic context while
 * executing `iret`) so it can continue transparently at the previous position.
 */
struct InterruptContext {
	// Scratch register (stored by `interrupt/handler.asm`)
	uintptr_t r11;         ///< scratch register R11
	uintptr_t r10;         ///< scratch register R10
	uintptr_t r9;          ///< scratch register R9
	uintptr_t r8;          ///< scratch register R8
	uintptr_t rdi;         ///< scratch register RDI
	uintptr_t rsi;         ///< scratch register RSI
	uintptr_t rdx;         ///< scratch register RDX
	uintptr_t rcx;         ///< scratch register RCX
	uintptr_t rax;         ///< scratch register RAX

	// Context saved by CPU
	uintptr_t error_code;  ///< Error Code
	uintptr_t ip;          ///< Instruction Pointer (at interrupt)
	uintptr_t cs : 16;     ///< Code segment (in case of a ring switch it is the segment of the user mode)
	uintptr_t    : 0;      ///< Alignment (due to 16 bit code segment)
	uintptr_t flags;       ///< Status flags register
	uintptr_t sp;          ///< Stack pointer (at interrupt)
	uintptr_t ss : 16;     ///< Stack segment (in case of a ring switch it is the segment of the user mode)
	uintptr_t    : 0;      ///< Alignment (due to 16 bit stack segment)
} __attribute__((packed));

/*! \brief High-Level Interrupt Handling.
 *  \ingroup interrupts
 *
 * Main interrupt handling routine of the system.
 * This function is called by the corresponding `interrupt_entry_VECTOR`
 * function (located in `interrupt/handler.asm`) with disabled interrupts.
 *
 *  \param vector number of the interrupt
 *  \param context Pointer to interrupt context (on stack).
 *
 */
extern "C" void interrupt_handler(Core::Interrupt::Vector vector, InterruptContext *context);

/*! \brief Array of function pointer to the default low-level interrupt handlers
 *
 * The index corresponds to the vectors entry function, e.g. `interrupt_entry[6]`
 * points to `interrupt_entry_6`, handling the trap for
 * \ref Core::Interrupt::INVALID_OPCODE "invalid opcode".
 *
 * The entry functions and this array are defined in assembly in
 * `interrupt/handler.asm` and used in \ref kernel_init() to
 * initialize the \ref IDT "Interrupt Descriptor Table (IDT)".
 */
extern "C" void * const interrupt_entry[];
