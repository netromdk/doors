OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

PHDRS
{
  text PT_LOAD FLAGS(5);  /* PF_R | PF_X = 4 | 1 */
  data PT_LOAD FLAGS(6);  /* PF_R | PF_W = 4 | 2 */
}

SECTIONS
{
  . = 0x10000000;

  .text : {
    *(.text)
    *(.text.*)
  } :text

  .rodata : {
    *(.rodata)
    *(.rodata.*)
  } :text

  .data : {
    *(.data)
    *(.data.*)
  } :data

  .bss : {
    *(COMMON)
    *(.bss)
    *(.bss.*)
  } :data

  /DISCARD/ : {
    *(.comment)
    *(.note.*)
  }
}
