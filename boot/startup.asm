; This is the actual entry point of the kernel.
; The switch into the 32-bit 'Protected Mode' has already been performed
; (by the boot loader).
; The assembly code just performs the absolute necessary steps (like setting up
; the stack) to be able to jump into the C++ code -- and continue further
; initialization in a (more) high-level language.

[BITS 32]

; External functions and variables
[EXTERN CPU_CORE_STACK_SIZE]        ; Constant containing the initial stack size (per CPU core), see `machine/core.cc`
[EXTERN cpu_core_stack_pointer]     ; Pointer to reserved memory for CPU core stacks, see `machine/core.cc`
[EXTERN gdt_protected_mode_pointer] ; Pointer to 32 Bit Global Descriptor Table (located in `machine/gdt.cc`)
[EXTERN long_mode]                  ; Low level function to jump into the 64-bit mode ('Long Mode', see `boot/longmode.asm`)
[EXTERN multiboot_addr]             ; Variable, in which the Pointer to Multiboot information
                                    ; structure should be stored (`boot/multiboot/data.cc`)

; Load Multiboot settings
%include "boot/multiboot/config.inc"

[SECTION .text]

; Entry point for the bootstrap processor (CPU0)
[GLOBAL startup_bsp]
startup_bsp:
	; Check if kernel was booted by a Multiboot compliant boot loader
	cmp eax, MULTIBOOT_HEADER_MAGIC_LOADER
	jne skip_multiboot
	; Pointer to Multiboot information structure has been stored in ebx by the
	; boot loader -- copy to a variable for later usage.
	mov [multiboot_addr], ebx

skip_multiboot:
	; Disable interrupts
	cli
	; Disable non maskable interrupts (NMI)
	; (we are going to ignore them)
	mov al, 0x80
	out 0x70, al

	jmp load_cs

; Segment initialization
; (code used by bootstrap and application processors as well)
[GLOBAL segment_init]
segment_init:
	; Load temporary protected mode Global Descriptor Table (GDT)
	lgdt [gdt_protected_mode_pointer]

	; Initialize segment register
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Load code segment register
	jmp 0x8:load_cs

load_cs:
	; Initialize stack pointer:
	; Atomic increment of `cpu_core_stack_pointer` by `CPU_CORE_STACK_SIZE`
	; (to avoid race conditions at application processor boot)
	mov eax, [CPU_CORE_STACK_SIZE]
	lock xadd [cpu_core_stack_pointer], eax
	; Since the stack grows into the opposite direction,
	; Add `CPU_CORE_STACK_SIZE` again
	add eax, [CPU_CORE_STACK_SIZE]
	; Assign stack pointer
	mov esp, eax

	; Clear direction flag for string operations
	cld

	; Switch to long mode (64 bit)
	jmp long_mode
