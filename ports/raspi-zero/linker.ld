ENTRY(_start)

SECTIONS
{
  . = 0x8000;

  .text : ALIGN(4) {
    KEEP(*(.init))
    *(.text*)
    *(.rodata*)
  }

  .data : ALIGN(4) {
    *(.data*)
  }

  .bss : ALIGN(4) {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  }

  . = ALIGN(16);
  . += 0x4000;
  __stack_top = .;
}
