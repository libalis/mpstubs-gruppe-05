;
; Copyright (C) 2016  Matt Borgerson
; Copyright (C) 2019  Bernhard Heinloth
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
;

%define NUM_HANDLERS 40

[SECTION .text]
; The generic GDB interrupt handler is (similar to the default interrupt_handler) a high-level C++ function
[EXTERN gdb_interrupt_handler]

; Entry function for GDB interrupt handling

; Similar to the entry function in `interrupt/handler.asm`,
; however, this function has to save more (all) registers.
;
; Usage: IRQ <vector> <error-code?>
%macro DBGIRQ 2
ALIGN 16
gdb_interrupt_entry_%1:
	; CPU saves
	; - FLAGS
	; - CS
	; - IP
	; - ERROR CODE, if provided (for vector 8, 10 - 14 and 17)
	%if %2 == 0
		; If the vector is not a trap with an error code automatically pushed
		; on the stack, the entry function pushes a zero instead
		; to retain an identical stack layout in each case.
		push 0
	%endif

	; - VECTOR (interrupt number)
	push %1

	; Store general-purpose register
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	; and segment register
	push fs
	push gs

	; context (pointer to current stack) as first (and only) parameter
	mov rdi, rsp

	; call the GDB interrupt handler
	call gdb_interrupt_handler

	; restore context from stack
	pop gs
	pop fs

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax

	add rsp, 16 ; due to ERROR CODE and VECTOR
	iretq
%endmacro

; Call macro for each trap (but can also be defined manually, like in the `interrupt/handler.asm`)
%assign i 0
%rep NUM_HANDLERS
	%if (i == 8) || ((i >= 10) && (i <= 14)) || (i == 17)
		; with ERROR CODE
		DBGIRQ i, 1
	%else
		; without ERROR CODE
		DBGIRQ i, 0
	%endif
	%assign i i+1
%endrep

[SECTION .data]

; Create a table (array) with function pointers of the specific GDB interrupt entry functions
[GLOBAL gdb_interrupt_entry]

gdb_interrupt_entry:
%macro gdb_interrupt_vector 1
	dq gdb_interrupt_entry_%1
%endmacro
%assign i 0
%rep NUM_HANDLERS
	gdb_interrupt_vector i
	%assign i i+1
%endrep

; Store the number of GDB interrupt entry functions in a variable
[GLOBAL gdb_interrupt_entries]

gdb_interrupt_entries:
	dd NUM_HANDLERS
