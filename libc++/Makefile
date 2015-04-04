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

CFLAGS:=$(CFLAGS) -Wall -Wextra -fno-builtin
CPPFLAGS:=$(CPPFLAGS) -D__IS_DOORS_LIBCPP -Iinclude -I$(PWD)/sysroot/$(INCLUDEDIR)/include
LIBK_CFLAGS:=$(CFLAGS) -ffreestanding -fno-exceptions -fno-rtti
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__IS_DOORS_KERNEL
LIBCPPTEST_CFLAGS:=$(CFLAGS) 
LIBCPPTEST_CPPFLAGS:=$(CPPFLAGS) -D__IS_DOORS_LIBCPPTEST

ARCHDIR:=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LIBCPPTEST_CFLAGS:=$(LIBCPPTEST_CFLAGS)
LIBCPPTEST_CPFLAGS:=$(LIBCPPTEST_CPPFLAGS)

FREEOBJS:=\
$(ARCH_FREEOBJS) \
stdio/putchar.o \
stdio/puts.o \
assert/assertFail.o \
stdlib/abort.o \
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
ctype/isalnum.o \
ctype/isalpha.o \
ctype/isblank.o \
ctype/iscntrl.o \
ctype/isdigit.o \
ctype/isxdigit.o \
ctype/isgraph.o \
ctype/ispunct.o \
ctype/isspace.o \
ctype/islower.o \
ctype/isupper.o \
ctype/isprint.o \
ctype/tolower.o \
ctype/toupper.o

HOSTEDOBJS:=\
$(ARCH_HOSTEDOBJS) \

OBJS:=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS:=$(FREEOBJS:.o=.libk.o)

LIBCPPTEST_OBJS:=$(FREEOBJS:.o=.libc++test.o)

BINARIES=libc++.a libk.a
TESTINGBINS=libc++test.a

all: $(BINARIES)
test: $(TESTINGBINS)

.PHONY: all clean install install-headers install-libs

libc++.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)
	@echo "[Created libc++.a]\n"

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)
	@echo "[Created libk.a]\n"

libc++test.a: $(LIBCPPTEST_OBJS)
	$(AR_HOST) rcs $@ $(LIBCPPTEST_OBJS)
	@echo "[Created libc++test.a]\n"

%.o: %.cpp
	$(CXX) -c $< -o $@ -std=c++11 $(CFLAGS) $(CPPFLAGS)

%.o: %.s
	$(CXX) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.libk.o: %.cpp
	$(CXX) -c $< -o $@ -std=c++11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libk.o: %.s
	$(CXX) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libc++test.o: %.cpp
	$(CXX_HOST) -c $< -o $@ -std=c++11 $(LIBCPPTEST_CFLAGS) $(LIBCPPTEST_CPPFLAGS)

%.libc++test.o: %.s
	$(CXX_HOST) -c $< -o $@ $(LIBCPPTEST_CFLAGS) $(LIBCPPTEST_CPPFLAGS)

clean:
	rm -f $(BINARIES) $(TESTINGBINS) $(OBJS) $(LIBK_OBJS) $(LIBCPPTEST_OBJS) *.o */*.o */*/*.o

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
