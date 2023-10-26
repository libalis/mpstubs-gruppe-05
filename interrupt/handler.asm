; Low-level stuff required for interrupt handling
; The "actual" code to be executed is implemented in the C function "interrupt_handler"

[EXTERN interrupt_handler]

[SECTION .text]

; Entry function for interrupt handling (of any vector)

; The interrupt handling of each vector has to be started in assembler to store
; the scratch registers (see SystemV calling conventions) before the actual
; high-level (C++) interrupt_handler function can be executed.
;
; For this purpose we use a macro to generate a customized entry function for
; each interrupt vector (0-255), wheres the vector itself is the first parameter.
; The second parameter is a boolean flag indicating whether an error code is
; placed on the stack for the corresponding trap (by the CPU).
;
; Usage: IRQ <vector> <error-code?>
%macro IRQ 2
ALIGN 16
interrupt_entry_%1:
	%if %2 == 0
		; If the vector is not a trap with an error code automatically pushed
		; on the stack, the entry function pushes a zero instead
		; to retain an identical stack layout in each case.
		push 0
	%endif

	; The interrupt may be triggered asynchronously, therefore the whole context
	; has to be saved and restored, or the interrupted code might not be able to
	; continue. The C++ compiler will only generates code to preserve
	; non-scratch registers in the high-level interrupt handler -- the scratch
	; registers have to be saved (and restored later) manually!
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11

	; Clear direction flag for string operations
	cld

	; Assign vector as first parameter for the high-level interrupt handler
	mov rdi, %1

	; Assign pointer to the context (= interrupt stack) as second parameter
	mov rsi, rsp

	; Call the high-level interrupt handler routine
	call interrupt_handler

	; Restore scratch registers
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax

	; Drop error code (or the fake zero value)
	add rsp, 8

	; Return from interrupt
	iretq
%endmacro

; For traps the CPU sometimes pushes an error code onto the stack.
; These vectors are documented in the manual (or instead use the
; [osdev wiki](https://wiki.osdev.org/Exceptions) for a handy list).
; Therefore we manually call the macro for the corresponding traps.
IRQ  0, 0
IRQ  1, 0
IRQ  2, 0
IRQ  3, 0
IRQ  4, 0
IRQ  5, 0
IRQ  6, 0
IRQ  7, 0
IRQ  8, 1
IRQ  9, 0
IRQ 10, 1
IRQ 11, 1
IRQ 12, 1
IRQ 13, 1
IRQ 14, 1
IRQ 15, 0
IRQ 16, 0
IRQ 17, 1

; All subsequent interrupts (18 - 255) have no error code,
; therefore we use a loop calling the macro.
%assign i 18
%rep 238
IRQ i, 0
%assign i i+1
%endrep

[SECTION .data]

; Create a function pointer array for each interrupt entry
; (to be used in C++ for IDT::handle)
[GLOBAL interrupt_entry]

interrupt_entry:
%macro interrupt_vector 1
	dq interrupt_entry_%1
%endmacro
%assign i 0
%rep 256
	interrupt_vector i
	%assign i i+1
%endrep
