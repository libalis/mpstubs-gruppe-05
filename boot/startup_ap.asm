; Startup of the remaining application processors (in real mode)
; and switching to 'Protected Mode' with a temporary GDT.
; This code is relocated by ApplicationProcessor::relocateSetupCode()

[SECTION .setup_ap_seg]
[GLOBAL setup_ap_gdt]
[GLOBAL setup_ap_gdtd]

; Unlike the bootstrap processor, the application processors have not been
; setup by the boot loader -- they start in real mode (16 bit) and have to be
; switched manually to protected mode (32 bit)
[BITS 16]

setup_ap:
	; Initialize segment register
	mov ax, cs ; Code segment and...
	mov ds, ax ; .. data segment should point to the same segment
	; (we don't use stack / stack segment)

	; Disable interrupts
	cli
	; Disable non maskable interrupts (NMI)
	mov al, 0x80
	out 0x70, al

	; load temporary real mode Global Descriptor Table (GDT)
	lgdt [setup_ap_gdtd - setup_ap]

	; Switch to protected mode:
	; enable protected mode bit (1 << 0) in control register 0
	mov eax, cr0
	or  eax, 1
	mov cr0, eax
	; Far jump to 32 bit `startup_ap` function
	jmp dword 0x08:startup_ap

; memory reserved for temporary real mode GDT
; initialized by ApplicationProcessor::relocateSetupCode()
ALIGN 4
setup_ap_gdt:
	dq 0,0,0,0,0  ; reserve memory for at least 5 GDT entries

; memory reserved for temporary real mode GDT descriptor
; initialized by ApplicationProcessor::relocateSetupCode()
setup_ap_gdtd:
	dw 0,0,0,0,0  ; reserve memory for GDT descriptor

[SECTION .text]

[BITS 32]

; Segment initialization defined in `boot/startup.asm`
[EXTERN segment_init]

; protected mode (32 bit) startup code for application processor
startup_ap:
	; reload all segment selectors (since they still point to the real mode GDT)
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Use same segment initialization function as bootstrap processor
	jmp segment_init
