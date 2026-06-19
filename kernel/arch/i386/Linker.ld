/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

  /* Read-write data (initialized).
     IMPORTANT: `.data.rel.ro` contains UBSAN type metadata (type_descriptor,
     source_location, etc.) when KERNEL_UBSAN is enabled. Without an explicit
     rule, the linker makes these sections orphans, placing them after
     `_kernel_end` where they overlap the heap. The bootloader's LOAD segment
     will NOT cover them, so the metadata is never loaded into memory, causing
     every UBSAN check to read uninitialized data, resulting in false-positive
     panics. */
  .data BLOCK(4K) : ALIGN(4K)
  {
    *(.data .data.*)
    *(.data.rel.ro .data.rel.ro.*)
  }

  /* Global Offset Table: used by position-independent code and also by
     GCC's UBSAN instrumentation for certain type info references. */
  .got BLOCK(4K) : ALIGN(4K)
  {
    *(.got)
    *(.got.plt)
  }

  /* ELF init/fini arrays: called by `__do_global_ctors_aux` / `__do_global_dtors_aux`.
     NOTE: Door's `_init` currently has a `ret` before `__do_global_ctors_aux`,
     so they are dead code. They are included to avoid orphan placement. */
  .init_array BLOCK(4K) : ALIGN(4K)
  {
    __init_array_start = .;
    *(.init_array .init_array.*)
    __init_array_end = .;
  }

  .fini_array BLOCK(4K) : ALIGN(4K)
  {
    __fini_array_start = .;
    *(.fini_array .fini_array.*)
    __fini_array_end = .;
  }

  /* GCC constructor/destructor tables (used by `__do_global_ctors_aux`). */
  .ctors BLOCK(4K) : ALIGN(4K)
  {
    *(.ctors .ctors.*)
  }

  .dtors BLOCK(4K) : ALIGN(4K)
  {
    *(.dtors .dtors.*)
  }

  /* Read-write data (uninitialized) and stack. */
  .bss BLOCK(4K) : ALIGN(4K)
  {
    *(COMMON)
    *(.bss)
    *(.bootstrap_stack)
  }

  . = ALIGN(16);
  _kernel_end = .;

  /* The compiler may produce other sections, by default it will put them in
     a segment with the same name. Simply add stuff here as needed. */
}
