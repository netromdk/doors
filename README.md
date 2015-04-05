# Doors
The meaning of the name is the recursive accronym "Doors of Open Run-time Systems".

# Concept

The challenge was to write a 32-bit OS using C++11 aiming for paged memory, exceptions/interrupts support, and a keyboard driver. Then dropping into a simple shell for processing simple commands like querying CPU information, uptime, memory available/used, start/stop/monitor timers etc. Maybe being able to play a simple "Snake" game or similar.

Things to look into later:
* Floating-point number support ([IEEE-754](https://en.wikipedia.org/wiki/IEEE_754-1985))
* Multi task scheduling
* Real-Time Clock/CMOS support
* Harddisk driver (supporting FAT32 or EXT2 to make it possible to install to disk and boot from it)
* Mouse driver
* USB drivers (general concept)

# Usage

The buid system uses Makefiles with the following usage (`make help`):
```
=== Doors Makefile ===
Development: build, test, tags, clean-tests, clean-tags, clean, clean-all
Emulation: run, run-iso
Distribution: iso, zip, tgz, bz2, xz
```

Before you `build` take a look at "config.sh". It will use the `ARCH-ABI-*` GNU programs, e.g. i386-elf-g++, i386-elf-as, i386-elf-ar etc. Right nown there is only support for i386.

`run` will build and use QEmu to run the kernel directly.

`run-iso` will build, create ISO and use QEmu to load the ISO as a CD-ROM. Note that to create the ISO it uses `grub-mkrescue` which requires [GRUB 2+](https://www.gnu.org/software/grub/) and [xorriso 1.3.8+](https://www.gnu.org/software/xorriso/). 

# References
* [Intel i386 manuals](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html) ([PDF](http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-manual-325462.pdf))
* [Logix's i386 reference](http://www.logix.cz/michal/doc/i386/)
* [osdev.org](http://wiki.osdev.org/Main_Page)
