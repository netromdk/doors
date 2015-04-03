TESTS = test_libc_string_strlen \
test_libc_string_strcpy

CPPFLAGS += -isystem=../sysroot/usr/include
CXXFLAGS += -g -Wall -Wextra

LIBC_LIB = ../libc/libctest.a

help:
	@echo "Run 'make test' in parent directory."

clean:
	rm -f $(TESTS)

run: $(TESTS)
	sh -c 'for t in $(TESTS); do `pwd`/runtest.sh $$t; done'

### libc/string ###
test_libc_string_strlen: libc/string/strlen.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strlen.cpp $(LIBC_LIB) -o $@

test_libc_string_strcpy: libc/string/strcpy.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strcpy.cpp $(LIBC_LIB) -o $@
