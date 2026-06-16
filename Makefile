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

test:
	@./scripts/buildtests.sh
	@./scripts/runtests.sh

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
check-qemu:
	@which $(EMU) > /dev/null 2>&1 || { \
		echo "Error: $(EMU) not found."; \
		echo "Install it with: sudo apt install qemu-system-x86"; \
		exit 1; \
	}

check-iso-deps:
	@which grub-mkrescue > /dev/null 2>&1 || { \
		echo "Error: grub-mkrescue not found."; \
		echo "Install it with: sudo apt install grub-pc-bin grub-common"; \
		exit 1; \
	}
	@which mformat > /dev/null 2>&1 || { \
		echo "Error: mformat not found (required by grub-mkrescue)."; \
		echo "Install it with: sudo apt install mtools"; \
		exit 1; \
	}
	@test -d /usr/lib/grub/i386-pc || { \
		echo "Error: GRUB i386-pc BIOS modules not found (required for bootable ISO)."; \
		echo "Install them with: sudo apt install grub-pc-bin"; \
		exit 1; \
	}

run: check-qemu build
	$(EMU) -kernel $(KERNEL)

run-iso: check-qemu iso
	$(EMU) -cdrom $(ISO)

# === Distribution ===
iso: check-iso-deps build
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
