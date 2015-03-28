KERNEL=kernel/buros.kernel
ISO=buros.iso
EMU=qemu-system-i386
TMPISODIR=/tmp/burosiso

help:
	@echo "BurOS Makefile"
	@echo "Targets: build, run, iso, run-iso, clean"

build:
	@./build.sh

run:
	$(EMU) -kernel $(KERNEL)

iso:
	rm -fr $(TMPISODIR)
	mkdir -p $(TMPISODIR)/boot/grub
	cp $(KERNEL) $(TMPISODIR)/boot
	cp grub.cfg $(TMPISODIR)/boot/grub
	grub-mkrescue -o $(ISO) --locale-directory=. $(TMPISODIR)
	rm -fr $(TMPISODIR)

run-iso:
	$(EMU) -cdrom $(ISO)

clean:
	@echo "Cleaning up.."
	@./clean.sh
