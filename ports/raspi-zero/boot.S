.section .init, "ax"
.global _start
_start:
    ldr sp, =__stack_top
    bl kernel_main
1:
    wfe
    b 1b
