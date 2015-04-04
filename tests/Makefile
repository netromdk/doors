TESTS = test_libc_string_strlen \
test_libc_string_strcmp \
test_libc_string_strncmp \
test_libc_string_strcpy \
test_libc_string_strncpy

CXXFLAGS += -isystem=../sysroot/usr/include -Wall -Wextra -std=c++11

LIBC_LIB = ../libc/libctest.a

help:
	@echo "Run 'make test' in parent directory."

clean:
	rm -fr $(TESTS) *.dSYM

run: $(TESTS)
	sh -c 'for t in $(TESTS); do `pwd`/runtest.sh $$t; done'

### libc/string ###
test_libc_string_strlen: libc/string/strlen.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strlen.cpp $(LIBC_LIB) $(CXXFLAGS) -o $@

test_libc_string_strcmp: libc/string/strcmp.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strcmp.cpp $(LIBC_LIB) $(CXXFLAGS) -o $@

test_libc_string_strncmp: libc/string/strncmp.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strncmp.cpp $(LIBC_LIB) $(CXXFLAGS) -o $@

test_libc_string_strcpy: libc/string/strcpy.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strcpy.cpp $(LIBC_LIB) $(CXXFLAGS) -o $@

test_libc_string_strncpy: libc/string/strncpy.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strncpy.cpp $(LIBC_LIB) $(CXXFLAGS) -o $@
