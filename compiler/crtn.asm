; C Runtime Objects - Function prologues for the initialization (.init) and termination routines (.fini) required by the runtime libs.

; C Runtime - end (needs to be passed as last element to the linker)
[SECTION .init]
	; The linker will inject the contents of the .init from crtend.o here
	pop rbp
	ret

[SECTION .fini]
	; The linker will inject the contents of the .fini from crtend.o here
	pop rbp
	ret
