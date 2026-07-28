OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

PHDRS
{
  text PT_LOAD FLAGS(5);  /* PF_R | PF_X */
}

SECTIONS
{
  /* Above KERNEL_VIRTUAL_BASE (0xC0000000) means `ElfLoader::validateSegment()` rejects it. */
  . = 0xE0000000;

  .text : {
    *(.text)
    *(.text.*)
  } :text

  /DISCARD/ : {
    *(.comment)
    *(.note.*)
  }
}
