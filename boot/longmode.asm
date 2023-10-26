; The stony path to Long Mode (64-bit)...
; ... begins in 32-bit Protected Mode
[BITS 32]

; Pointer to Long Mode Global Descriptor Table (GDT, machine/gdt.cc)
[EXTERN gdt_long_mode_pointer]

[GLOBAL long_mode]
long_mode:

; You can check if the CPU supports Long Mode by using the `cpuid` command.
; Problem: You first have to figure out if the `cpuid` command itself is
; supported. Therefore, you have to try to reverse the 21st bit in the EFLAGS
; register -- if it works, then there is the 'cpuid' instruction.
CPUID_BIT_MASK equ 1 << 21

check_cpuid:
	; Save EFLAGS on stack
	pushfd

	; Copy stored EFLAGS from stack to EAX register
	mov eax, [esp]

	; Flip the 21st bit (ID) in EAX
	xor eax, CPUID_BIT_MASK

	; Copy EAX to EFLAGS (using the stack)
	push eax
	popfd

	; And reverse: copy EFLAGS to EAX (using the stack)
	; (but the 21st bit should now still be flipped, if `cpuid` is supported)
	pushfd
	pop eax

	; Compare the new EFLAGS copy (residing in EAX) with the EFLAGS stored at
	; the beginning of this function by using an exclusive OR -- all different
	; (flipped) bits will be stored in EAX.
	xor eax, [esp]

	; Restore original EFLAGS
	popfd

	; If 21st Bit in EAX is set, `cpuid` is supported -- continue at check_long_mode
	and eax, CPUID_BIT_MASK
	jnz check_long_mode

	; Show error message "No CPUID" and stop CPU
	mov dword [0xb8000], 0xcf6fcf4e
	mov dword [0xb8004], 0xcf43cf20
	mov dword [0xb8008], 0xcf55cf50
	mov dword [0xb800c], 0xcf44cf49
	hlt

; Now you are able to use the `cpuid` instruction to check if Long Mode is
; available -- after you've checked if the `cpuid` is able to perform the
; check itself (since it is an extended `cpuid` function)...

CPUID_GET_LARGEST_EXTENDED_FUNCTION_NUMBER equ 0x80000000
CPUID_GET_EXTENDED_PROCESSOR_FEATURES equ 0x80000001
CPUID_HAS_LONGMODE equ 1 << 29

check_long_mode:
	; Set argument for `cpuid` to check the availability of extended functions
	; and call cpuid
	mov eax, CPUID_GET_LARGEST_EXTENDED_FUNCTION_NUMBER
	cpuid
	; The return value contains the maximum function number supported by `cpuid`,
	; You'll need the function number for extended processor features
	cmp eax, CPUID_GET_EXTENDED_PROCESSOR_FEATURES
	; If not present, the CPU is definitely too old to support long mode
	jb no_long_mode

	; Finally, you are able to check the Long Mode support itself
	mov eax, CPUID_GET_EXTENDED_PROCESSOR_FEATURES
	cpuid
	; If the return value in the EDX register has set the 29th bit,
	; then long mode is supported -- continue with setup_paging
	test edx, CPUID_HAS_LONGMODE
	jnz setup_paging

no_long_mode:
	; Show error message "No 64bit" and stop CPU
	mov dword [0xb8000], 0xcf6fcf4e
	mov dword [0xb8004], 0xcf36cf20
	mov dword [0xb8008], 0xcf62cf34
	mov dword [0xb800c], 0xcf74cf69
	hlt

; Paging is required for Long Mode.
; Since an extensive page manager might be a bit of an overkill to start with,
; the following code creates an identity mapping for the first four gigabytes
; (using huge pages): each virtual address will point to the same physical one.
; This area (up to 4 GiB) is important for some memory mapped devices (APIC)
; and you don't want to remap them yet for simplicity reasons.
; In the advanced operating systems lecture, this topic is covered in detail,
; however, if you want a quick overview, have a look at
; https://wiki.osdev.org/Page_Tables#2_MiB_pages_2

PAGE_SIZE equ 4096
PAGE_FLAGS_PRESENT equ 1 << 0
PAGE_FLAGS_WRITEABLE equ 1 << 1
PAGE_FLAGS_USER equ 1 << 2
PAGE_FLAGS_HUGE equ 1 << 7

setup_paging:
	; Unlike in Protected Mode, an entry in the page table has a size of 8 bytes
	; (vs 4 bytes), so there are only 512 (and not 1024) entries per table.
	; Structure of the 3-level PAE paging: One entry in the
	; - lv2: Page-Directory-Table         (PDT)  covers 2 MiB   (1 Huge Page)
	; - lv3: Page-Directory-Pointer-Table (PDPT) covers 1 GiB   (512 * 2 MiB)
	; - lv4: Page-Map-Level-4-Table       (PML4) covers 512 GiB (512 * 1 GiB)

	; To address 4 GiB only four level-2 tables are required.
	; All entries of the level-2 tables should be marked as writeable (attributes)
	; and map (point to) the corresponding physical memory.

	; This is done in a loop using ECX as counter
	mov ecx, 0

.identitymap_level2:
	; Calculate physical address in EAX (2 MiB multiplied by the counter)
	mov eax, 0x200000
	mul ecx
	; Configure page attributes
	or eax, PAGE_FLAGS_PRESENT | PAGE_FLAGS_WRITEABLE | PAGE_FLAGS_HUGE | PAGE_FLAGS_USER
	; Write (8 byte) entry in the level-2 table
	mov [paging_level2_tables + ecx * 8], eax

	; Increment counter...
	inc ecx
	; ... until all four level-2 tables are filled
	cmp ecx, 512 * 4
	jne .identitymap_level2

	; The first four entries of the level-3 table should point to the
	; four level-2 tables (and be writeable as well).
	; Again, ECX acts as counter for the loop
	mov ecx, 0

.identitymap_level3:
	; Calculate the address: ECX * PAGE_SIZE + paging_level2_tables
	mov eax, ecx
	; The size of a page is stored in  the EDX register
	mov edx, PAGE_SIZE
	mul edx
	add eax, paging_level2_tables
	; Configure attributes
	or eax, PAGE_FLAGS_PRESENT | PAGE_FLAGS_WRITEABLE | PAGE_FLAGS_USER
	; Write (8 byte) entry in the level-3 table
	mov [paging_level3_table + ecx * 8], eax

	; Increment counter...
	inc ecx
	; ... until all four entries of the table are written
	cmp ecx, 4
	jne .identitymap_level3

	mov eax, paging_level2_tables
	or eax, PAGE_FLAGS_PRESENT | PAGE_FLAGS_WRITEABLE | PAGE_FLAGS_USER
	mov [paging_level3_table], eax

	; The first entry of the level-4 table should point to to the level-3 table
	mov eax, paging_level3_table
	or eax, PAGE_FLAGS_PRESENT | PAGE_FLAGS_WRITEABLE | PAGE_FLAGS_USER
	mov [paging_level4_table], eax

; Time to activate paging
paging_enable:
	; First setup the control registers

	; Write the address of the level-4 table into the CR3 register
	mov eax, paging_level4_table
	mov cr3, eax

	; Activate Physical Address Extension (PAE)
	; by setting the 5th bits in the CR4 register
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	; Set the Long Mode Enable Bit in the EFER MSR
	; (Extended Feature Enable Register Model Specific Register)
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	; Finally, the 31st bit in CR0 is set to enable Paging
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	; Load Long Mode Global Descriptor Table
	lgdt [gdt_long_mode_pointer]

	; Far jump to the 64-bit start code
	jmp 0x8:long_mode_start

	; print `KO` to screen
	mov dword [0xb8000], 0x3f4f3f4b
	hlt

; Memory reserved for page tables
[SECTION .bss]

ALIGN 4096

[GLOBAL paging_level4_table]
[GLOBAL paging_level3_table]
[GLOBAL paging_level2_tables]
; 1x Level-4 Table (Page Map Level 4)
paging_level4_table:
	resb PAGE_SIZE

; 1x Level-3 Table (Page Directory Pointer Table)
paging_level3_table:
	resb PAGE_SIZE

; 4x Level-2 Table (Page Directory)
paging_level2_tables:
	resb PAGE_SIZE * 4

[SECTION .text]
[EXTERN kernel_init]    ; C++ entry function

; Continue with 64 bit code
[BITS 64]

long_mode_start:
	; Set data segment registers to SEGMENT_KERNEL_DATA (machine/gdt.h)
	mov ax, 0x10
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Call high-level (C++) kernel initialization function
	call kernel_init

	; Print `STOP` to screen and stop
	mov rax, 0x2f502f4f2f544f53
	mov qword [0xb8000], rax
	hlt
