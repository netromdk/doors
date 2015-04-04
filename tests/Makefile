TESTS = test_libc_string_memcpy \
test_libc_string_memset \
test_libc_string_memcmp \
test_libc_string_memmove \
test_libc_string_memchr \
test_libc_string_strlen \
test_libc_string_strcmp \
test_libc_string_strncmp \
test_libc_string_strcpy \
test_libc_string_strncpy \
test_libc_string_strchr \
test_libc_string_strrchr \
test_libc_stdlib_itos \
test_libc_stdlib_utos \
test_libc_stdlib_ltos \
test_libc_stdio_putchar \
test_libc_stdio_puts \
test_libc_stdio_printf \
test_libc_algorithm_min \
test_libc_algorithm_max \
test_libc_algorithm_swap

CXXFLAGS += -I../sysroot/$(INCLUDEDIR)/include -std=c++11 -O0 -Wall -Wextra -fno-builtin

LIBC_LIB = ../libc/libctest.a

help:
	@echo "Run 'make test' in parent directory."

clean:
	rm -fr $(TESTS) *.dSYM

run: $(TESTS)
	sh -c '`pwd`/scripts/pretest.sh; \
        for t in $(TESTS); \
          do `pwd`/scripts/runtest.sh $$t; \
        done; \
        `pwd`/scripts/posttest.sh'

### libc/string ###
test_libc_string_memcpy: libc/string/memcpy.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/memcpy.cpp -o $@

test_libc_string_memset: libc/string/memset.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/memset.cpp -o $@

test_libc_string_memcmp: libc/string/memcmp.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/memcmp.cpp -o $@

test_libc_string_memmove: libc/string/memmove.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/memmove.cpp -o $@

test_libc_string_memchr: libc/string/memchr.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/memchr.cpp -o $@

test_libc_string_strlen: libc/string/strlen.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/strlen.cpp -o $@

test_libc_string_strcmp: libc/string/strcmp.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/strcmp.cpp -o $@

test_libc_string_strncmp: libc/string/strncmp.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/strncmp.cpp -o $@

test_libc_string_strcpy: libc/string/strcpy.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/strcpy.cpp -o $@

test_libc_string_strncpy: libc/string/strncpy.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/strncpy.cpp -o $@

test_libc_string_strchr: libc/string/strchr.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/strchr.cpp -o $@

test_libc_string_strrchr: libc/string/strrchr.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/string/strrchr.cpp -o $@

### libc/stdlib ###
test_libc_stdlib_itos: libc/stdlib/itos.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/stdlib/itos.cpp -o $@

test_libc_stdlib_utos: libc/stdlib/utos.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/stdlib/utos.cpp -o $@

test_libc_stdlib_ltos: libc/stdlib/ltos.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/stdlib/ltos.cpp -o $@

### libc/stdio ###
test_libc_stdio_putchar: libc/stdio/putchar.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/stdio/putchar.cpp -o $@

test_libc_stdio_puts: libc/stdio/puts.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/stdio/puts.cpp -o $@

test_libc_stdio_printf: libc/stdio/printf.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/stdio/printf.cpp -o $@

### lib/algorithm ###
test_libc_algorithm_min: libc/algorithm/min.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/algorithm/min.cpp -o $@

test_libc_algorithm_max: libc/algorithm/max.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/algorithm/max.cpp -o $@

test_libc_algorithm_swap: libc/algorithm/swap.cpp $(LIBC_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBC_LIB) libc/algorithm/swap.cpp -o $@
