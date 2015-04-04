TESTS = test_libc++_string_memcpy \
test_libc++_string_memset \
test_libc++_string_memcmp \
test_libc++_string_memmove \
test_libc++_string_memchr \
test_libc++_string_strlen \
test_libc++_string_strcmp \
test_libc++_string_strncmp \
test_libc++_string_strcpy \
test_libc++_string_strncpy \
test_libc++_string_strchr \
test_libc++_string_strrchr \
test_libc++_stdlib_itos \
test_libc++_stdlib_utos \
test_libc++_stdlib_ltos \
test_libc++_stdio_putchar \
test_libc++_stdio_puts \
test_libc++_stdio_printf \
test_libc++_algorithm_min \
test_libc++_algorithm_max \
test_libc++_algorithm_swap \
test_libc++_ctype_isalnum \
test_libc++_ctype_isalpha

CXXFLAGS += -I../sysroot/$(INCLUDEDIR)/include -std=c++11 -O0 -Wall -Wextra -fno-builtin

LIBCPP_LIB = ../libc++/libc++test.a

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

### libc++/string ###
test_libc++_string_memcpy: libc++/string/memcpy.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/memcpy.cpp -o $@

test_libc++_string_memset: libc++/string/memset.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/memset.cpp -o $@

test_libc++_string_memcmp: libc++/string/memcmp.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/memcmp.cpp -o $@

test_libc++_string_memmove: libc++/string/memmove.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/memmove.cpp -o $@

test_libc++_string_memchr: libc++/string/memchr.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/memchr.cpp -o $@

test_libc++_string_strlen: libc++/string/strlen.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/strlen.cpp -o $@

test_libc++_string_strcmp: libc++/string/strcmp.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/strcmp.cpp -o $@

test_libc++_string_strncmp: libc++/string/strncmp.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/strncmp.cpp -o $@

test_libc++_string_strcpy: libc++/string/strcpy.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/strcpy.cpp -o $@

test_libc++_string_strncpy: libc++/string/strncpy.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/strncpy.cpp -o $@

test_libc++_string_strchr: libc++/string/strchr.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/strchr.cpp -o $@

test_libc++_string_strrchr: libc++/string/strrchr.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/string/strrchr.cpp -o $@

### libc++/stdlib ###
test_libc++_stdlib_itos: libc++/stdlib/itos.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/stdlib/itos.cpp -o $@

test_libc++_stdlib_utos: libc++/stdlib/utos.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/stdlib/utos.cpp -o $@

test_libc++_stdlib_ltos: libc++/stdlib/ltos.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/stdlib/ltos.cpp -o $@

### libc++/stdio ###
test_libc++_stdio_putchar: libc++/stdio/putchar.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/stdio/putchar.cpp -o $@

test_libc++_stdio_puts: libc++/stdio/puts.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/stdio/puts.cpp -o $@

test_libc++_stdio_printf: libc++/stdio/printf.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/stdio/printf.cpp -o $@

### libc++/algorithm ###
test_libc++_algorithm_min: libc++/algorithm/min.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/algorithm/min.cpp -o $@

test_libc++_algorithm_max: libc++/algorithm/max.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/algorithm/max.cpp -o $@

test_libc++_algorithm_swap: libc++/algorithm/swap.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/algorithm/swap.cpp -o $@

### libc++/ctype ###
test_libc++_ctype_isalnum: libc++/ctype/isalnum.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/ctype/isalnum.cpp -o $@

test_libc++_ctype_isalpha: libc++/ctype/isalpha.cpp $(LIBCPP_LIB)
	$(CXX_HOST) $(CXXFLAGS) $(LIBCPP_LIB) libc++/ctype/isalpha.cpp -o $@
