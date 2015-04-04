TESTS = test_libc_string_strlen \
test_libc_string_strcmp \
test_libc_string_strncmp \
test_libc_string_strcpy \
test_libc_string_strncpy \
test_libc_string_strchr \
test_libc_string_strrchr

CXXFLAGS += -isystem=../sysroot/usr/include -Wall -Wextra -std=c++11

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

test_libc_string_strchr: libc/string/strchr.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strchr.cpp $(LIBC_LIB) $(CXXFLAGS) -o $@

test_libc_string_strrchr: libc/string/strrchr.cpp $(LIBC_LIB)
	$(CXX_HOST) libc/string/strrchr.cpp $(LIBC_LIB) $(CXXFLAGS) -o $@
