TESTS = test_libc_string_strlen

CPPFLAGS += -isystem=../sysroot/usr/include
CXXFLAGS += -g -Wall -Wextra

LIBC_LIB = ../libc/libctest.a

help:
	@echo "Run 'make test' in parent directory."

clean:
	rm -f $(TESTS)

run: $(TESTS)
	sh -c 'for t in $(TESTS); do `pwd`/runtest.sh $$t; done'

test_libc_string_strlen: libc/string/strlen.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strlen.cpp $(LIBC_LIB) -o $@
