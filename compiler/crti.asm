; C Runtime Objects - Function prologues for the initialization (.init) and termination routines (.fini) required by the runtime libs.
; When developing on Linux, these files are automatically added by the compiler/linker; we, however, are writing an operating system
; and therefore need to add them ourselves.

; C Runtime - beginning (needs to be passed as first element to the linker)
[SECTION .init]
[GLOBAL _init]
_init:
	push rbp
	mov rbp, rsp
	; The linker will inject the contents of the .init from crtbegin.o here

[SECTION .fini]
[GLOBAL _fini]
_fini:
	push rbp
	mov rbp, rsp
	; The linker will inject the contents of the .fini from crtbegin.o here
