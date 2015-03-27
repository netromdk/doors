AS=i386-elf-as
CXX=i386-elf-g++
CXXFLAGS=-ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
LDFLAGS=-ffreestanding -O2 -nostdlib -lgcc
EMU=qemu-system-i386
BIN=buros.bin
ISO=buros.iso
BUILDDIR=_build
TMPISODIR=/tmp/burosiso

help:
	@echo "BurOS Makefile"
	@echo "Targets: build, run, iso, run-iso, clean"

build:
	mkdir -p $(BUILDDIR)
	$(AS) boot/boot.s -o $(BUILDDIR)/boot.o
	$(CXX) $(CXXFLAGS) -c src/kernel.cpp -o $(BUILDDIR)/kernel.o
	$(CXX) $(CXXFLAGS) -c src/term.cpp -o $(BUILDDIR)/term.o
	$(CXX) $(CXXFLAGS) -c src/string.cpp -o $(BUILDDIR)/string.o
	$(CXX) -T linker.ld $(LDFLAGS) \
	  $(BUILDDIR)/boot.o \
	  $(BUILDDIR)/kernel.o \
          $(BUILDDIR)/term.o \
          $(BUILDDIR)/string.o \
	  -o $(BIN)

run:
	$(EMU) -kernel $(BIN)

iso:
	rm -fr $(TMPISODIR)
	mkdir -p $(TMPISODIR)/boot/grub
	cp $(BIN) $(TMPISODIR)/boot
	cp boot/grub.cfg $(TMPISODIR)/boot/grub
	grub-mkrescue -o $(ISO) --locale-directory=. $(TMPISODIR)
	rm -fr $(TMPISODIR)

run-iso:
	$(EMU) -cdrom $(ISO)

clean:
	@echo "Cleaning up.."
	@rm -frv $(BUILDDIR)
	@find . -iname '*~' | xargs rm -frv
	@find . -iname '*.bin' | xargs rm -frv
	@find . -iname '*.iso' | xargs rm -frv
