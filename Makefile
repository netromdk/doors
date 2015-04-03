KERNEL=kernel/doors.kernel
ISO=doors.iso
ZIP=doors.zip
TGZ=doors.tgz
BZ2=doors.bz2
XZ=doors.xz
EMU=qemu-system-i386
TMPISODIR=/tmp/doors_iso
SYSROOT=sysroot

help:
	@echo "=== Doors Makefile ==="
	@echo "Development: build, tags, clean-tags, clean, clean-all"
	@echo "Emulation: run, run-iso"
	@echo "Distribution: iso"

# === Development ===
build:
	@./scripts/build.sh

tags:
	@./scripts/gtags.sh

clean-tags:
	@echo "Cleaning up tags.."
	@rm -fv GTAGS GPATH GRTAGS

clean:
	@echo "Cleaning up.."
	@./scripts/clean.sh

clean-all: clean clean-tags

# === Emulation ===
run: build
	$(EMU) -kernel $(KERNEL)

run-iso: iso
	$(EMU) -cdrom $(ISO)

# === Distribution ===
iso: build
	rm -fr $(TMPISODIR)
	mkdir -p $(TMPISODIR)/boot/grub
	cp $(KERNEL) $(TMPISODIR)/boot
	cp grub.cfg $(TMPISODIR)/boot/grub
	grub-mkrescue -o $(ISO) --locale-directory=. $(TMPISODIR)
	rm -fr $(TMPISODIR)

zip: build
	rm -f $(ZIP)
	zip -r9 $(ZIP) $(SYSROOT)

tgz: build
	rm -f $(TGZ)
	tar -cvzf $(TGZ) $(SYSROOT)

bz2: build
	rm -f $(BZ2)
	tar -cvjf $(BZ2) $(SYSROOT)

xz: build
	rm -f $(XZ)
	tar -cvJf $(XZ) $(SYSROOT)
