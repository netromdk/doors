# Concept

The challenge was to write an OS using C++11 aiming for paged memory, exceptions/interrupts support, and a keyboard driver. Then dropping into a simple shell for processing simple commands. Maybe being able to play a simple "Snake" game or similar.

Things to look into later:
* Floating-point number support ([IEEE-754](https://en.wikipedia.org/wiki/IEEE_754-1985))
* Multi task scheduling
* Harddisk driver (supporting FAT32 or EXT2 to make it possible to install to disk and boot from it)
* Mouse driver
* USB drivers (general concept)

# References
* [Intel i386 manuals](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html) ([PDF](http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-manual-325462.pdf))
* [Logix's i386 reference](http://www.logix.cz/michal/doc/i386/)
* [OSDev.org](http://wiki.osdev.org/Main_Page)
