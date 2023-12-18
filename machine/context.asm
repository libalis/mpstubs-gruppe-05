[SECTION .text]
[GLOBAL context_switch]
; The current contexts register are saved on stack,
; and the next contexts register are read into the processor.

ALIGN 16
context_switch:
push r15
push r14
push r13
push r12
push rbp
push rbx
mov rdi, rsp
mov rsp, rsi ; !!!TODO!!!
pop rbx
pop rbp
pop r12
pop r13
pop r14
pop r15
ret
