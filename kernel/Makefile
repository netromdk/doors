HOST?=$(shell ../scripts/default-host.sh)
HOSTARCH:=$(shell ../scripts/target-triplet-to-arch.sh $(HOST))

CFLAGS?=-O0
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__IS_DOORS_KERNEL -Iinclude -I$(PWD)/sysroot/$(INCLUDEDIR)/include
LDFLAGS:=$(LDFLAGS) -L$(PWD)/sysroot/$(LIBDIR)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR:=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

OBJS:=\
$(KERNEL_ARCH_OBJS) \
kernel/kernel.o \
kernel/runtime.o \

CRTI_OBJ:=$(ARCHDIR)/crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtend.o)
CRTN_OBJ:=$(ARCHDIR)/crtn.o

ALL_OUR_OBJS:=\
$(OBJS) \
$(CRTI_OBJ) \
$(CRTN_OBJ) \

OBJ_LINK_LIST:=\
$(OBJS) \
$(CRTI_OBJ) \
$(CRTBEGIN_OBJ) \
$(CRTEND_OBJ) \
$(CRTN_OBJ) \

all: doors.kernel

.PHONY: all clean install install-headers install-kernel

doors.kernel: $(OBJ_LINK_LIST) $(ARCHDIR)/linker.ld
	$(CXX) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(OBJ_LINK_LIST) $(LDFLAGS) $(LIBS)
	@echo "[Linked doors.kernel]"

%.o: %.cpp
	$(CXX) -c $< -o $@ -std=c++11 $(CFLAGS) $(CPPFLAGS)

%.o: %.s
	$(CXX) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f doors.kernel $(OBJS) $(ALL_OUR_OBJS) *.o */*.o */*/*.o

install: install-headers install-kernel

install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@cp -R include $(DESTDIR)$(INCLUDEDIR)

install-kernel: doors.kernel
	@mkdir -p $(DESTDIR)$(BOOTDIR)
	@cp doors.kernel $(DESTDIR)$(BOOTDIR)
