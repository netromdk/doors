KERNEL=kernel/doors.kernel
ISO=doors.iso
EMU=qemu-system-i386
TMPISODIR=/tmp/doors_iso

help:
	@echo "=== Doors Makefile ==="
	@echo "Development: build, tags, clean-tags, clean, clean-all"
	@echo "Emulation: run, run-iso"
	@echo "Distribution: iso"

build:
	@./scripts/build.sh

run: build
	$(EMU) -kernel $(KERNEL)

iso: build
	rm -fr $(TMPISODIR)
	mkdir -p $(TMPISODIR)/boot/grub
	cp $(KERNEL) $(TMPISODIR)/boot
	cp grub.cfg $(TMPISODIR)/boot/grub
	grub-mkrescue -o $(ISO) --locale-directory=. $(TMPISODIR)
	rm -fr $(TMPISODIR)

run-iso: iso
	$(EMU) -cdrom $(ISO)

tags:
	@./scripts/gtags.sh

clean-tags:
	@echo "Cleaning up tags.."
	@rm -fv GTAGS GPATH GRTAGS

clean:
	@echo "Cleaning up.."
	@./scripts/clean.sh

clean-all: clean clean-tags
