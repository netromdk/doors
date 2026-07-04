OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
  . = 0x10000000;

  .text : {
    *(.text)
    *(.text.*)
  }

  .rodata : {
    *(.rodata)
    *(.rodata.*)
  }

  .data : {
    *(.data)
    *(.data.*)
  }

  .bss : {
    *(COMMON)
    *(.bss)
    *(.bss.*)
  }

  /DISCARD/ : {
    *(.comment)
    *(.note.*)
  }
}
