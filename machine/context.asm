[SECTION .text]
[GLOBAL context_switch]
; The current contexts register are saved on stack,
; and the next contexts register are read into the processor.

ALIGN 16
context_switch:
