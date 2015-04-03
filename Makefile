NAME=doors
KERNEL=kernel/$(NAME).kernel
ISO=$(NAME).iso
ZIP=$(NAME).zip
TGZ=$(NAME).tgz
BZ2=$(NAME).bz2
XZ=$(NAME).xz
EMU=qemu-system-i386
TMPISODIR=/tmp/$(NAME)_iso
SYSROOT=sysroot

help:
	@echo "=== Doors Makefile ==="
	@echo "Development: build, test, tags, clean-tests, clean-tags, clean, clean-all"
	@echo "Emulation: run, run-iso"
	@echo "Distribution: iso, zip, tgz, bz2, xz"

# === Development ===
build:
	@./scripts/build.sh

test: build
	make -C tests run

clean-tests:
	make -C tests clean

tags:
	@./scripts/gtags.sh

clean-tags:
	@echo "Cleaning up tags.."
	@rm -fv GTAGS GPATH GRTAGS

clean:
	@echo "Cleaning up.."
	@./scripts/clean.sh

clean-all: clean clean-tests clean-tags

# === Emulation ===
run: build
	$(EMU) -kernel $(KERNEL)

run-iso: iso
	$(EMU) -cdrom $(ISO)

# === Distribution ===
iso: build
	@echo "Creating distribution: $(ISO)"
	@rm -fr $(TMPISODIR)
	@mkdir -p $(TMPISODIR)/boot/grub
	@cp $(KERNEL) $(TMPISODIR)/boot
	@cp grub.cfg $(TMPISODIR)/boot/grub
	@grub-mkrescue -o $(ISO) --locale-directory=. $(TMPISODIR)
	@rm -fr $(TMPISODIR)

zip: build
	@echo "Creating distribution: $(ZIP)"
	@rm -f $(ZIP)
	@zip -r9 $(ZIP) $(SYSROOT)

tgz: build
	@echo "Creating distribution: $(TGZ)"
	@rm -f $(TGZ)
	@tar -cvzf $(TGZ) $(SYSROOT)

bz2: build
	@echo "Creating distribution: $(BZ2)"
	@rm -f $(BZ2)
	@tar -cvjf $(BZ2) $(SYSROOT)

xz: build
	@echo "Creating distribution: $(XZ)"
	@rm -f $(XZ)
	@tar -cvJf $(XZ) $(SYSROOT)
