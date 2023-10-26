; Magic Header, has to be present in Kernel to indicate Multiboot compliance
MULTIBOOT_HEADER_MAGIC_OS equ 0x1badb002

; Answer by the boot loader for Multiboot compliance, written in eax register
MULTIBOOT_HEADER_MAGIC_LOADER equ 0x2badb002

; Flags instructing the Multiboot compliant boot loader to setup the system
; according to your needs
MULTIBOOT_PAGE_ALIGN  equ 1<<0    ; Align boot modules (initrds) at 4 KiB border
MULTIBOOT_MEMORY_INFO equ 1<<1    ; Request Memory Map information
MULTIBOOT_VIDEO_MODE  equ 1<<2    ; Configure video mode

MULTIBOOT_HEADER_FLAGS equ 0

; Desired video mode (only considered if MULTIBOOT_VIDEO_MODE set)
; (boot loader will choose the best fitting mode, which might differ from the settings below)
MULTIBOOT_VIDEO_WIDTH equ 1280    ; Desired width
MULTIBOOT_VIDEO_HEIGHT equ 1024   ; Desired height
MULTIBOOT_VIDEO_BITDEPTH equ 32   ; Desired bit depth

; Checksum
MULTIBOOT_HEADER_CHKSUM equ -(MULTIBOOT_HEADER_MAGIC_OS + MULTIBOOT_HEADER_FLAGS)
