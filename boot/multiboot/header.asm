; The first 8192 bytes of the kernel binary must contain a header with
; predefined (and sometimes "magic") values according to the Multiboot standard.
; Based on these values, the boot loader decides whether and how to load the
; kernel -- which is compiled and linked into an ELF file.
; To make this possible with your StuBS kernel, the linker places the following
; entry `multiboot_header` at the very beginning of the file thanks to the
; linker script (located in compiler/sections.ld).

[SECTION .multiboot_header]

; Include configuration
%include 'boot/multiboot/config.inc'

; Multiboot Header
ALIGN 4
multiboot_header:
	dd MULTIBOOT_HEADER_MAGIC_OS  ; Magic Header Value
	dd MULTIBOOT_HEADER_FLAGS     ; Flags (affects following entries)
	dd MULTIBOOT_HEADER_CHKSUM    ; Header Checksum

	; Following fields would have been required to be defined
	; if flag A_OUT KLUDGE was set (but we don't need this)
	dd 0     ; Header address
	dd 0     ; Begin of load  address
	dd 0     ; end of load address
	dd 0     ; end of bss segment
	dd 0     ; address of entry function

	; Following fields are required for video mode (flag MULTIBOOT_VIDEO_MODE)
	dd 0   ; Mode: 0 = Graphic / 1 = Text
	dd MULTIBOOT_VIDEO_WIDTH     ; Width (pixels / columns)
	dd MULTIBOOT_VIDEO_HEIGHT    ; Height (pixels / rows)
	dd MULTIBOOT_VIDEO_BITDEPTH  ; color depth / number of colors
