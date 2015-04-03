HOST?=$(shell ../scripts/default-host.sh)
HOSTARCH:=$(shell ../scripts/target-triplet-to-arch.sh $(HOST))

CFLAGS?=-O0
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_PREFIX)/lib

CFLAGS:=$(CFLAGS) -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__IS_DOORS_LIBC -Iinclude -I$(PWD)/sysroot/$(INCLUDEDIR)/include
LIBK_CFLAGS:=$(CFLAGS) -ffreestanding -fno-builtin -fno-exceptions -fno-rtti
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__IS_DOORS_KERNEL
LIBCTEST_CFLAGS:=$(CFLAGS)
LIBCTEST_CPPFLAGS:=$(CPPFLAGS) -D__IS_DOORS_LIBCTEST

ARCHDIR:=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LIBCTEST_CFLAGS:=$(LIBCTEST_CFLAGS)
LIBCTEST_CPFLAGS:=$(LIBCTEST_CPPFLAGS)

FREEOBJS:=\
$(ARCH_FREEOBJS) \
stdio/putchar.o \
stdio/puts.o \
stdio/printf.o \
stdlib/itos.o \
stdlib/utos.o \
stdlib/ltos.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/memcmp.o \
string/memchr.o \
string/strlen.o \
string/strcpy.o \
string/strncpy.o \
string/strcmp.o \
string/strncmp.o \
string/strchr.o \
string/strrchr.o \

HOSTEDOBJS:=\
$(ARCH_HOSTEDOBJS) \

OBJS:=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS:=$(FREEOBJS:.o=.libk.o)

LIBCTEST_OBJS:=$(FREEOBJS:.o=.libctest.o)

BINARIES=libc.a libk.a
TESTINGBINS=libctest.a

all: $(BINARIES)
test: $(TESTINGBINS)

.PHONY: all clean install install-headers install-libs

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)
	@echo "[Created libc.a]\n"

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)
	@echo "[Created libk.a]\n"

libctest.a: $(LIBCTEST_OBJS)
	$(AR_HOST) rcs $@ $(LIBCTEST_OBJS)
	@echo "[Created libctest.a]\n"

%.o: %.cpp
	$(CXX) -c $< -o $@ -std=c++11 $(CFLAGS) $(CPPFLAGS)

%.o: %.s
	$(CXX) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.libk.o: %.cpp
	$(CXX) -c $< -o $@ -std=c++11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libk.o: %.s
	$(CXX) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libctest.o: %.cpp
	$(CXX_HOST) -c $< -o $@ -std=c++11 $(LIBCTEST_CFLAGS) $(LIBCTEST_CPPFLAGS)

%.libctest.o: %.s
	$(CXX_HOST) -c $< -o $@ $(LIBCTEST_CFLAGS) $(LIBCTEST_CPPFLAGS)

clean:
	rm -f $(BINARIES) $(TESTINGBINS) $(OBJS) $(LIBK_OBJS) $(LIBCTEST_OBJS) *.o */*.o */*/*.o

install: install-headers install-libs

install-tests: install-headers install-testlibs

install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@cp -R include $(DESTDIR)$(INCLUDEDIR)

install-libs: $(BINARIES)
	@mkdir -p $(DESTDIR)$(LIBDIR)
	@cp $(BINARIES) $(DESTDIR)$(LIBDIR)

install-testlibs: $(TESTINGBINS)
	@mkdir -p $(DESTDIR)$(LIBDIR)
	@cp $(TESTINGBINS) $(DESTDIR)$(LIBDIR)
