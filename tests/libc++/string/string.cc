#include <doctest/doctest.h>
#include <cstring>
#include <string>

TEST_CASE("default constructor")
{
  string s;
  CHECK(s.empty());
  CHECK(s.size() == 0);
  CHECK(s.capacity() == string::SSO_CAPACITY);
  CHECK(strcmp(s.c_str(), "") == 0);
  CHECK(s.data() == s.c_str());
}

// SSO: Simple String Optimization.
TEST_CASE("c-string constructor: SSO")
{
  const string s("hello");
  CHECK(s.size() == 5);
  CHECK(s.capacity() == string::SSO_CAPACITY);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("c-string constructor: heap")
{
  const char *longStr = "this a very long string that exceeds SSO buffer capacity";
  const string s(longStr);
  CHECK(s.size() == strlen(longStr));
  CHECK(s.capacity() >= s.size());
  CHECK(strcmp(s.c_str(), longStr) == 0);
}

TEST_CASE("fill constructor: SSO")
{
  const string s(5, 'x');
  CHECK(s.size() == 5);
  CHECK(s.capacity() == string::SSO_CAPACITY);
  CHECK(strcmp(s.c_str(), "xxxxx") == 0);
}

TEST_CASE("fill constructor: heap")
{
  string s(100, 'x');
  CHECK(s.size() == 100);
  CHECK(s.capacity() >= s.size());
  for (size_t i = 0; i < 100; ++i) {
    CHECK(s[i] == 'x');
  }
}

TEST_CASE("copy constructor: SSO")
{
  const string a("hello");
  const string b(a); // NOLINT(performance-unnecessary-copy-initialization)
  CHECK(b.size() == 5);
  CHECK(strcmp(b.c_str(), "hello") == 0);
  CHECK(b.capacity() == string::SSO_CAPACITY);
}

TEST_CASE("copy constructor: heap")
{
  const char *heapStr = "this is a very long string that must be on the heap for sure";
  const string a(heapStr);
  const string b(a); // NOLINT(performance-unnecessary-copy-initialization)
  CHECK(b.size() == a.size());
  CHECK(strcmp(b.c_str(), a.c_str()) == 0);
  CHECK(b.capacity() >= b.size());
}

TEST_CASE("move constructor: SSO")
{
  string a("hello");
  const string b(static_cast<string &&>(a));
  CHECK(b.size() == 5);
  CHECK(strcmp(b.c_str(), "hello") == 0);
  CHECK(a.empty());
  CHECK(strcmp(a.c_str(), "") == 0);
}

TEST_CASE("move constructor: heap")
{
  const char *heapStr = "this string will be on the heap without any doubt at all";
  string a(heapStr);
  const char *origData = a.data();
  string b(static_cast<string &&>(a));
  CHECK(b.size() == strlen(heapStr));
  CHECK(b.data() == origData);
  CHECK(a.empty());
  CHECK(strcmp(a.c_str(), "") == 0);
}

TEST_CASE("copy assignment")
{
  const string a("world");
  string b("hello");
  b = a;
  CHECK(strcmp(b.c_str(), "world") == 0);
  CHECK(b.size() == 5);
}

TEST_CASE("copy assignment: heap to heap")
{
  const string a("this is a very long string that must be on the heap for sure");
  string b("another long string that should also be on the heap yes indeed");
  b = a;
  CHECK(strcmp(b.c_str(), a.c_str()) == 0);
  CHECK(b.size() == a.size());
}

TEST_CASE("move assignment")
{
  string a("hello");
  string b;
  b = static_cast<string &&>(a);
  CHECK(strcmp(b.c_str(), "hello") == 0);
  CHECK(a.empty());
}

TEST_CASE("move assignment: self assignment safe")
{
  string a("hello");
  string &ref = a;
  a = static_cast<string &&>(ref);
  CHECK(strcmp(a.c_str(), "hello") == 0);
}

TEST_CASE("assignment from c-string")
{
  string s;
  s = "hello world";
  CHECK(strcmp(s.c_str(), "hello world") == 0);
  CHECK(s.size() == 11);
}

TEST_CASE("SSO boundary: SSO_CAPACITY chars fits in SSO")
{
  const string s("12345678901234567890123");
  CHECK(s.size() == string::SSO_CAPACITY);
  CHECK(s.capacity() == string::SSO_CAPACITY);
}

TEST_CASE("SSO boundary: SSO_CAPACITY+1 chars goes to heap")
{
  const string s("123456789012345678901234");
  CHECK(s.size() == string::SSO_CAPACITY + 1);
  CHECK(s.capacity() >= string::SSO_CAPACITY + 1);
}

TEST_CASE("SSO to heap via push_back")
{
  string s("12345678901234567890123");
  s.push_back('6');
  CHECK(s.size() == string::SSO_CAPACITY + 1);
  CHECK(strcmp(s.c_str(), "123456789012345678901236") == 0);
}

TEST_CASE("SSO to heap via append")
{
  string s("hello");
  s.append(" world, this is a long suffix");
  CHECK(s.size() > string::SSO_CAPACITY);
  CHECK(strcmp(s.c_str(), "hello world, this is a long suffix") == 0);
}

TEST_CASE("SSO to heap via operator+=")
{
  string s("hello");
  s += " world, this is a long suffix";
  CHECK(s.size() > string::SSO_CAPACITY);
}

TEST_CASE("SSO to heap via reserve")
{
  string s("hello");
  s.reserve(100);
  CHECK(s.capacity() >= 100);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("shrink_to_fit from heap to heap")
{
  string s("hello world this is a test string");
  s.reserve(200);
  CHECK(s.capacity() >= 200);
  s.shrink_to_fit();
  CHECK(s.capacity() <= 50);
  CHECK(strcmp(s.c_str(), "hello world this is a test string") == 0);
}

TEST_CASE("shrink_to_fit from heap to SSO")
{
  string s("hello world this is a test");
  s = "hi";
  s.shrink_to_fit();
  CHECK(s.capacity() == string::SSO_CAPACITY);
  CHECK(strcmp(s.c_str(), "hi") == 0);
}

TEST_CASE("clear from heap")
{
  string s("a very long string that exceeds SSO buffer capacity");
  s.clear();
  CHECK(s.empty());
  CHECK(s.size() == 0);
  CHECK(strcmp(s.c_str(), "") == 0);
}

TEST_CASE("clear from SSO")
{
  string s("hello");
  s.clear();
  CHECK(s.empty());
  CHECK(s.size() == 0);
  CHECK(strcmp(s.c_str(), "") == 0);
}

TEST_CASE("operator[]")
{
  string s("hello");
  CHECK(s[0] == 'h');
  CHECK(s[1] == 'e');
  CHECK(s[2] == 'l');
  CHECK(s[3] == 'l');
  CHECK(s[4] == 'o');
}

TEST_CASE("at(): valid access")
{
  string s("hello");
  CHECK(s.at(0) == 'h');
  CHECK(s.at(4) == 'o');
}

TEST_CASE("front and back")
{
  string s("hello");
  CHECK(s.front() == 'h');
  CHECK(s.back() == 'o');
}

TEST_CASE("c_str null-terminated: SSO")
{
  const string s("hello");
  CHECK(s.c_str()[5] == '\0');
}

TEST_CASE("c_str null-terminated: heap")
{
  const string s("this is a long string that lives on the heap");
  CHECK(s.c_str()[s.size()] == '\0');
}

TEST_CASE("data equals c_str")
{
  string s("hello");
  CHECK(s.data() == s.c_str());
  string t("a very long string that exceeds SSO buffer capacity");
  CHECK(t.data() == t.c_str());
}

TEST_CASE("empty")
{
  const string s;
  CHECK(s.empty());
  const string t("hello");
  CHECK(!t.empty());
}

TEST_CASE("size and length")
{
  const string s("hello");
  CHECK(s.size() == 5);
  CHECK(s.length() == 5);
}

TEST_CASE("reserve pre-allocates")
{
  string s;
  s.reserve(100);
  CHECK(s.capacity() >= 100);
  CHECK(s.empty());
}

TEST_CASE("insert at beginning")
{
  string s("world");
  s.insert(0, "hello ");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("insert in middle")
{
  string s("helorld");
  s.insert(3, "lo w");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("insert at end")
{
  string s("hello");
  s.insert(5, " world");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("erase from beginning")
{
  string s("hello world");
  s.erase(0, 6);
  CHECK(strcmp(s.c_str(), "world") == 0);
}

TEST_CASE("erase from middle")
{
  string s("hello world");
  s.erase(5, 1);
  CHECK(strcmp(s.c_str(), "helloworld") == 0);
}

TEST_CASE("erase from end")
{
  string s("hello world");
  s.erase(5);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("erase entire string")
{
  string s("hello");
  s.erase(0, string::npos);
  CHECK(s.empty());
}

TEST_CASE("push_back SSO to SSO")
{
  string s("hello");
  s.push_back('!');
  CHECK(s.size() == 6);
  CHECK(strcmp(s.c_str(), "hello!") == 0);
}

TEST_CASE("push_back SSO to heap")
{
  string s("12345678901234567890123");
  s.push_back('6');
  CHECK(s.size() == string::SSO_CAPACITY + 1);
  CHECK(strcmp(s.c_str(), "123456789012345678901236") == 0);
}

TEST_CASE("push_back heap to heap")
{
  string s("123456789012345678901234");
  s.push_back('7');
  CHECK(s.size() == string::SSO_CAPACITY + 2);
  CHECK(strcmp(s.c_str(), "1234567890123456789012347") == 0);
}

TEST_CASE("pop_back")
{
  string s("hello");
  s.pop_back();
  CHECK(s.size() == 4);
  CHECK(strcmp(s.c_str(), "hell") == 0);
  s.pop_back();
  CHECK(s.size() == 3);
  CHECK(strcmp(s.c_str(), "hel") == 0);
}

TEST_CASE("pop_back empty")
{
  string s;
  s.pop_back();
  CHECK(s.empty());
}

TEST_CASE("append string")
{
  string s("hello");
  s.append(string(" world"));
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("append c-string")
{
  string s("hello");
  s.append(" world");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("operator+= string")
{
  string s("hello");
  s += string(" world");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("operator+= char")
{
  string s("hello");
  s += '!';
  CHECK(strcmp(s.c_str(), "hello!") == 0);
}

TEST_CASE("resize truncate")
{
  string s("hello world");
  s.resize(5);
  CHECK(s.size() == 5);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("resize extend with null")
{
  string s("hello");
  s.resize(10);
  CHECK(s.size() == 10);
  CHECK(s[5] == '\0');
  CHECK(s[6] == '\0');
  CHECK(s[9] == '\0');
  CHECK(s.c_str()[10] == '\0');
}

TEST_CASE("resize extend with char")
{
  string s("hello");
  s.resize(10, 'x');
  CHECK(s.size() == 10);
  CHECK(strcmp(s.c_str(), "helloxxxxx") == 0);
}

TEST_CASE("swap SSO SSO")
{
  string a("hello");
  string b("world");
  a.swap(b);
  CHECK(strcmp(a.c_str(), "world") == 0);
  CHECK(strcmp(b.c_str(), "hello") == 0);
}

TEST_CASE("swap heap heap")
{
  string a("this is a very long string on the heap");
  string b("another long heap string here as well");
  const string aOrig(a);
  const string bOrig(b);
  a.swap(b);
  CHECK(strcmp(a.c_str(), bOrig.c_str()) == 0);
  CHECK(strcmp(b.c_str(), aOrig.c_str()) == 0);
}

TEST_CASE("swap SSO heap")
{
  string a("hello");
  string b("this is a string on the heap for swapping");
  const string aOrig(a);
  const string bOrig(b);
  a.swap(b);
  CHECK(strcmp(a.c_str(), bOrig.c_str()) == 0);
  CHECK(strcmp(b.c_str(), aOrig.c_str()) == 0);
}

TEST_CASE("find char from start")
{
  const string s("hello world");
  CHECK(s.find('h') == 0);
  CHECK(s.find('o') == 4);
  CHECK(s.find('w') == 6);
}

TEST_CASE("find char from pos")
{
  const string s("hello world");
  CHECK(s.find('o', 5) == 7);
}

TEST_CASE("find char not found")
{
  const string s("hello");
  CHECK(s.find('z') == string::npos);
}

TEST_CASE("find string from start")
{
  const string s("hello world");
  CHECK(s.find(string("world")) == 6);
  CHECK(s.find(string("hello")) == 0);
}

TEST_CASE("find string not found")
{
  const string s("hello world");
  CHECK(s.find(string("xyz")) == string::npos);
}

TEST_CASE("rfind char from end")
{
  const string s("hello world");
  CHECK(s.rfind('l') == 9);
  CHECK(s.rfind('d') == 10);
}

TEST_CASE("rfind char not found")
{
  const string s("hello");
  CHECK(s.rfind('z') == string::npos);
}

TEST_CASE("find_first_of")
{
  const string s("hello world");
  CHECK(s.find_first_of("ow") == 4);
}

TEST_CASE("starts_with string")
{
  const string s("hello world");
  CHECK(s.starts_with(string("hello")));
  CHECK(!s.starts_with(string("world")));
}

TEST_CASE("starts_with char")
{
  const string s("hello");
  CHECK(s.starts_with('h'));
  CHECK(!s.starts_with('x'));
}

TEST_CASE("ends_with string")
{
  const string s("hello world");
  CHECK(s.ends_with(string("world")));
  CHECK(!s.ends_with(string("hello")));
}

TEST_CASE("ends_with char")
{
  const string s("hello");
  CHECK(s.ends_with('o'));
  CHECK(!s.ends_with('x'));
}

TEST_CASE("compare equal")
{
  const string a("hello");
  const string b("hello");
  CHECK(a.compare(b) == 0);
}

TEST_CASE("compare less")
{
  const string a("apple");
  const string b("banana");
  CHECK(a.compare(b) < 0);
}

TEST_CASE("compare greater")
{
  const string a("banana");
  const string b("apple");
  CHECK(a.compare(b) > 0);
}

TEST_CASE("operator==")
{
  const string a("hello");
  string b("hello");
  const string c("world");
  CHECK(a == b);
  CHECK(!(a == c));
}

TEST_CASE("operator!=")
{
  const string a("hello");
  string b("world");
  CHECK(a != b);
}

TEST_CASE("operator<")
{
  const string a("apple");
  string b("banana");
  CHECK(a < b);
  CHECK(!(b < a));
}

TEST_CASE("operator>")
{
  const string a("banana");
  string b("apple");
  CHECK(a > b);
}

TEST_CASE("operator<=")
{
  const string a("apple");
  string b("apple");
  CHECK(a <= b);
  CHECK(a <= string("banana"));
}

TEST_CASE("operator>=")
{
  const string a("banana");
  string b("banana");
  CHECK(a >= b);
  CHECK(a >= string("apple"));
}

TEST_CASE("empty string c_str")
{
  const string s;
  CHECK(strcmp(s.c_str(), "") == 0);
}

TEST_CASE("assign empty string")
{
  string s("hello");
  s = "";
  CHECK(s.empty());
  CHECK(s.size() == 0);
  CHECK(strcmp(s.c_str(), "") == 0);
}

TEST_CASE("single char string")
{
  string s("x");
  CHECK(s.size() == 1);
  CHECK(s[0] == 'x');
}

TEST_CASE("very long string")
{
  string s;
  for (int i = 0; i < 1000; ++i) {
    s.push_back('a');
  }
  CHECK(s.size() == 1000);
  CHECK(s.capacity() >= 1000);
  for (size_t i = 0; i < 1000; ++i) {
    CHECK(s[i] == 'a');
  }
  CHECK(s.c_str()[1000] == '\0');
}

TEST_CASE("embedded null character")
{
  char data[] = "he\0llo!";
  string s(data, 7);
  CHECK(s.size() == 7);
  CHECK(s[0] == 'h');
  CHECK(s[1] == 'e');
  CHECK(s[2] == '\0');
  CHECK(s[3] == 'l');
  CHECK(s[4] == 'l');
  CHECK(s[5] == 'o');
  CHECK(s[6] == '!');
  CHECK(s.c_str()[7] == '\0');
}

TEST_CASE("self copy assignment")
{
  string s("hello");
  const string &ref = s;
  s = ref;
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("multiple moves")
{
  string a("hello world");
  string b(static_cast<string &&>(a));
  const string c(static_cast<string &&>(b));
  CHECK(strcmp(c.c_str(), "hello world") == 0);
  CHECK(a.empty());
  CHECK(b.empty());
}

TEST_CASE("operator+ string string")
{
  const string a("hello ");
  const string b("world");
  const string c = a + b;
  CHECK(strcmp(c.c_str(), "hello world") == 0);
}

TEST_CASE("operator+ string cstr")
{
  const string a("hello ");
  const string c = a + "world";
  CHECK(strcmp(c.c_str(), "hello world") == 0);
}

TEST_CASE("operator+ cstr string")
{
  const string b(" world");
  const string c = "hello" + b;
  CHECK(strcmp(c.c_str(), "hello world") == 0);
}

TEST_CASE("SSO strings on stack: no fragmentation")
{
  const string a("one");
  const string b("two");
  const string c("three");
  const string d("four");
  const string e("five");
  CHECK(strcmp(a.c_str(), "one") == 0);
  CHECK(strcmp(b.c_str(), "two") == 0);
  CHECK(strcmp(c.c_str(), "three") == 0);
  CHECK(strcmp(d.c_str(), "four") == 0);
  CHECK(strcmp(e.c_str(), "five") == 0);
}

TEST_CASE("string as local variable in loop")
{
  for (int i = 0; i < 100; ++i) {
    const string s("temporary");
    CHECK(strcmp(s.c_str(), "temporary") == 0);
  }
}

TEST_CASE("fill constructor: count zero")
{
  const string s(0, 'x');
  CHECK(s.empty());
}

TEST_CASE("substring constructor with null")
{
  const string s("hello", 5);
  CHECK(s.size() == 5);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("begin/end SSO")
{
  string s("hello");
  size_t count = 0;
  for (auto it = s.begin(); it != s.end(); ++it) {
    ++count;
  }
  CHECK(count == 5);
  CHECK(*s.begin() == 'h');
}

TEST_CASE("begin/end heap")
{
  string s("this is a long string on the heap for sure");
  size_t count = 0;
  for (auto it = s.begin(); it != s.end(); ++it) {
    ++count;
  }
  CHECK(count == s.size());
}

TEST_CASE("begin/end empty")
{
  string s;
  CHECK(s.begin() == s.end());
}

TEST_CASE("const iterators")
{
  const string s("hello");
  size_t count = 0;
  for (auto it = s.cbegin(); it != s.cend(); ++it) {
    ++count;
  }
  CHECK(count == 5);
}

TEST_CASE("range-for loop SSO")
{
  const string s("abc");
  string result;
  for (const char c : s) {
    result.push_back(c);
  }
  CHECK(strcmp(result.c_str(), "abc") == 0);
}

TEST_CASE("range-for loop heap")
{
  const string s("a long string to iterate over the heap");
  size_t count = 0;
  for (const char c : s) {
    (void) c;
    ++count;
  }
  CHECK(count == s.size());
}

TEST_CASE("range-for loop empty")
{
  const string s;
  for (const char c : s) {
    (void) c;

    // Should not execute. The loop body must not be entered for empty string.
    CHECK(false);
  }
}

TEST_CASE("contains char")
{
  const string s("hello world");
  CHECK(s.contains('h'));
  CHECK(s.contains('d'));
  CHECK(s.contains(' '));
  CHECK(!s.contains('z'));
}

TEST_CASE("contains string")
{
  const string s("hello world");
  CHECK(s.contains(string("world")));
  CHECK(s.contains(string("hello")));
  CHECK(!s.contains(string("xyz")));
}

TEST_CASE("contains empty string")
{
  const string s;
  CHECK(!s.contains('a'));
  CHECK(!s.contains(string("anything")));
}

TEST_CASE("substr from start")
{
  const string s("hello world");
  const string sub = s.substr(0, 5);
  CHECK(strcmp(sub.c_str(), "hello") == 0);
}

TEST_CASE("substr from middle")
{
  const string s("hello world");
  const string sub = s.substr(6);
  CHECK(strcmp(sub.c_str(), "world") == 0);
}

TEST_CASE("substr entire string")
{
  const string s("hello");
  const string sub = s.substr(0);
  CHECK(strcmp(sub.c_str(), "hello") == 0);
}

TEST_CASE("substr empty result")
{
  const string s("hello");
  const string sub = s.substr(3, 0);
  CHECK(sub.empty());
}

TEST_CASE("substr beyond end")
{
  const string s("hello");
  const string sub = s.substr(100);
  CHECK(sub.empty());
}

TEST_CASE("substr heap")
{
  const string s("this string is on the heap for the substr test");
  const string sub = s.substr(5, 10);
  CHECK(sub.size() == 10);
  CHECK(memcmp(sub.c_str(), s.c_str() + 5, 10) == 0);
}

TEST_CASE("data non-const write")
{
  string s("hello");
  s.data()[0] = 'H';
  CHECK(strcmp(s.c_str(), "Hello") == 0);
}

TEST_CASE("front/back const access")
{
  const string s("hello");
  CHECK(s.front() == 'h');
  CHECK(s.back() == 'o');
}

TEST_CASE("max_size")
{
  const string s;
  CHECK(s.max_size() > 0);
  CHECK(s.max_size() == static_cast<string::size_type>(-1) / 2);
}

TEST_CASE("rfind string")
{
  const string s("hello world world");
  CHECK(s.rfind(string("world")) == 12);
  CHECK(s.rfind(string("hello")) == 0);
  CHECK(s.rfind(string("xyz")) == string::npos);
}

TEST_CASE("rfind c-string")
{
  const string s("hello world world");
  CHECK(s.rfind("world") == 12);
  CHECK(s.rfind("hello") == 0);
  CHECK(s.rfind("xyz") == string::npos);
}

TEST_CASE("rfind string with pos")
{
  const string s("hello world world");
  CHECK(s.rfind(string("world"), 11) == 6);
}

TEST_CASE("find_first_of string")
{
  const string s("hello world");
  CHECK(s.find_first_of(string("wo")) == 4);
}

TEST_CASE("find_first_of char")
{
  const string s("hello world");
  CHECK(s.find_first_of('o') == 4);
  CHECK(s.find_first_of('z') == string::npos);
}

TEST_CASE("find_first_of char from pos")
{
  const string s("hello world");
  CHECK(s.find_first_of('o', 5) == 7);
}

TEST_CASE("find_last_of char")
{
  const string s("hello world");
  CHECK(s.find_last_of('o') == 7);
  CHECK(s.find_last_of('z') == string::npos);
}

TEST_CASE("find_last_of string")
{
  const string s("hello world");
  CHECK(s.find_last_of(string("lo")) == 9);
}

TEST_CASE("find_last_of c-string")
{
  const string s("hello world");
  CHECK(s.find_last_of("lo") == 9);
}

TEST_CASE("find_last_of empty string")
{
  const string s;
  CHECK(s.find_last_of('a') == string::npos);
}

TEST_CASE("find_first_not_of char")
{
  const string s("aaab");
  CHECK(s.find_first_not_of('a') == 3);
  CHECK(s.find_first_not_of('b') == 0);
}

TEST_CASE("find_first_not_of string")
{
  const string s("hello world");
  CHECK(s.find_first_not_of(string("helo ")) == 6);
}

TEST_CASE("find_first_not_of c-string")
{
  const string s("hello world");
  CHECK(s.find_first_not_of("helo ") == 6);
}

TEST_CASE("find_first_not_of all match")
{
  const string s("aaaa");
  CHECK(s.find_first_not_of('a') == string::npos);
}

TEST_CASE("find_last_not_of char")
{
  const string s("baaa");
  CHECK(s.find_last_not_of('a') == 0);
}

TEST_CASE("find_last_not_of string")
{
  const string s("hello world");
  CHECK(s.find_last_not_of(string("world ")) == 1);
}

TEST_CASE("find_last_not_of c-string")
{
  const string s("hello world");
  CHECK(s.find_last_not_of("world ") == 1);
}

TEST_CASE("find_last_not_of all match")
{
  const string s("aaaa");
  CHECK(s.find_last_not_of('a') == string::npos);
}

TEST_CASE("replace in middle")
{
  string s("heXXXorld");
  s.replace(2, 3, "llo w");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("replace at start")
{
  string s("XXX world");
  s.replace(0, 3, "hello");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("replace at end")
{
  string s("hello XXX");
  s.replace(6, 3, "world");
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("replace entire string")
{
  string s("old content");
  s.replace(0, s.size(), "new content");
  CHECK(strcmp(s.c_str(), "new content") == 0);
}

TEST_CASE("replace with c-string")
{
  string s("abc");
  s.replace(0, 3, "xyz");
  CHECK(strcmp(s.c_str(), "xyz") == 0);
}

TEST_CASE("replace with string object")
{
  string s("abc");
  const string repl("xyz");
  s.replace(0, 3, repl);
  CHECK(strcmp(s.c_str(), "xyz") == 0);
}

TEST_CASE("copy to buffer")
{
  const string s("hello");
  char buf[10] = {};
  const size_t n = s.copy(buf, 5);
  CHECK(n == 5);
  CHECK(memcmp(buf, "hello", 5) == 0);
}

TEST_CASE("copy partial from pos")
{
  const string s("hello world");
  char buf[10] = {};
  const size_t n = s.copy(buf, 5, 6);
  CHECK(n == 5);
  CHECK(memcmp(buf, "world", 5) == 0);
}

TEST_CASE("copy beyond end")
{
  const string s("hello");
  char buf[10] = {};
  const size_t n = s.copy(buf, 10, 100);
  CHECK(n == 0);
}

TEST_CASE("copy exceeding size")
{
  const string s("hello");
  char buf[10] = {};
  const size_t n = s.copy(buf, 10, 2);
  CHECK(n == 3);
  CHECK(buf[0] == 'l');
  CHECK(buf[1] == 'l');
  CHECK(buf[2] == 'o');
}

TEST_CASE("non-member swap SSO SSO")
{
  string a("hello");
  string b("world");
  swap(a, b);
  CHECK(strcmp(a.c_str(), "world") == 0);
  CHECK(strcmp(b.c_str(), "hello") == 0);
}

TEST_CASE("non-member swap heap heap")
{
  string a("this is a long string on the heap");
  string b("another long heap string here as well");
  const string aOrig(a);
  const string bOrig(b);
  swap(a, b);
  CHECK(strcmp(a.c_str(), bOrig.c_str()) == 0);
  CHECK(strcmp(b.c_str(), aOrig.c_str()) == 0);
}

TEST_CASE("non-member swap SSO heap")
{
  string a("hello");
  string b("this is a string on the heap for swapping");
  const string aOrig(a);
  const string bOrig(b);
  swap(a, b);
  CHECK(strcmp(a.c_str(), bOrig.c_str()) == 0);
  CHECK(strcmp(b.c_str(), aOrig.c_str()) == 0);
}

TEST_CASE("starts_with on empty string")
{
  const string s;
  CHECK(!s.starts_with('a'));
  CHECK(!s.starts_with(string("a")));
}

TEST_CASE("ends_with on empty string")
{
  const string s;
  CHECK(!s.ends_with('a'));
  CHECK(!s.ends_with(string("a")));
}

TEST_CASE("starts_with with empty needle")
{
  const string s("hello");
  CHECK(s.starts_with(string("")));
}

TEST_CASE("ends_with with empty needle")
{
  const string s("hello");
  CHECK(s.ends_with(string("")));
}

TEST_CASE("substr with embedded nulls")
{
  char data[] = "he\0llo!";
  const string s(data, 7);
  string sub = s.substr(2, 2);
  CHECK(sub.size() == 2);
  CHECK(sub[0] == '\0');
  CHECK(sub[1] == 'l');
}

TEST_CASE("compare with embedded nulls")
{
  char data1[] = "he\0llo";
  char data2[] = "he\0xxx";
  const string a(data1, 6);
  const string b(data2, 6);
  CHECK(a.compare(b) < 0);
}

TEST_CASE("find empty string always returns pos")
{
  const string s("hello");
  CHECK(s.find(string(""), 2) == 2);
  CHECK(s.find(string(""), 100) == string::npos);
}

TEST_CASE("find at position beyond size")
{
  const string s("hello");
  CHECK(s.find('a', 100) == string::npos);
  CHECK(s.find(string("anything"), 100) == string::npos);
}

TEST_CASE("reserve no-arg shrinks heap")
{
  string s("hello world this is a test string");
  s.reserve(200);
  CHECK(s.capacity() >= 200);
  s.reserve();
  CHECK(s.capacity() < 200);
  CHECK(strcmp(s.c_str(), "hello world this is a test string") == 0);
}

TEST_CASE("reserve no-arg SSO unchanged")
{
  string s("hello");
  size_t cap = s.capacity();
  s.reserve();
  CHECK(s.capacity() == cap);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("insert repeated char at beginning")
{
  string s("llo");
  s.insert(0, 2, 'h');
  CHECK(strcmp(s.c_str(), "hhllo") == 0);
}

TEST_CASE("insert repeated char in middle")
{
  string s("heo");
  s.insert(2, 2, 'l');
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("insert repeated char at end")
{
  string s("hello");
  s.insert(5, 3, '!');
  CHECK(strcmp(s.c_str(), "hello!!!") == 0);
}

TEST_CASE("insert repeated char zero count")
{
  string s("hello");
  s.insert(2, 0, 'x');
  CHECK(strcmp(s.c_str(), "hello") == 0);
  CHECK(s.size() == 5);
}

TEST_CASE("insert repeated char heap")
{
  string s("a long string to insert into");
  s.insert(1, 100, '-');
  CHECK(s.size() > 20);
  CHECK(s[1] == '-');
  CHECK(s[100] == '-');
}

TEST_CASE("copy on empty string")
{
  const string s;
  char buf[10] = {0};
  const size_t n = s.copy(buf, 10);
  CHECK(n == 0);
}

TEST_CASE("erase with pos beyond end")
{
  string s("hello");
  s.erase(100);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("erase with zero count")
{
  string s("hello");
  s.erase(2, 0);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("erase with count exceeding size")
{
  string s("hello");
  s.erase(2, 100);
  CHECK(strcmp(s.c_str(), "he") == 0);
}

TEST_CASE("spaceship equal")
{
  const string a("hello");
  const string b("hello");
  CHECK((a <=> b) == 0);
  CHECK((a <=> b) >= 0);
  CHECK((a <=> b) <= 0);
}

TEST_CASE("spaceship less")
{
  const string a("apple");
  const string b("banana");
  CHECK((a <=> b) < 0);
  CHECK(!((a <=> b) > 0));
}

TEST_CASE("spaceship greater")
{
  const string a("banana");
  const string b("apple");
  CHECK((a <=> b) > 0);
  CHECK(!((a <=> b) < 0));
}

TEST_CASE("spaceship with prefix")
{
  const string a("abc");
  const string b("abcdef");
  CHECK((a <=> b) < 0);
  CHECK((b <=> a) > 0);
}

TEST_CASE("spaceship equal heap")
{
  const string a("a long string that lives on the heap for testing");
  const string b("a long string that lives on the heap for testing");
  CHECK((a <=> b) == 0);
}

TEST_CASE("erase free removes all occurrences")
{
  string s("hello world");
  const size_t n = erase(s, 'l');
  CHECK(n == 3);
  CHECK(strcmp(s.c_str(), "heo word") == 0);
}

TEST_CASE("erase free no match")
{
  string s("hello");
  const size_t n = erase(s, 'z');
  CHECK(n == 0);
  CHECK(strcmp(s.c_str(), "hello") == 0);
}

TEST_CASE("erase free all removed")
{
  string s("aaaaa");
  const size_t n = erase(s, 'a');
  CHECK(n == 5);
  CHECK(s.empty());
}

TEST_CASE("erase free empty string")
{
  string s;
  const size_t n = erase(s, 'a');
  CHECK(n == 0);
  CHECK(s.empty());
}

TEST_CASE("erase_if removes matching chars")
{
  string s("a1b2c3");
  const size_t n = erase_if(s, [](char c) { return c >= '0' && c <= '9'; });
  CHECK(n == 3);
  CHECK(strcmp(s.c_str(), "abc") == 0);
}

TEST_CASE("find 3-arg finds counted substring")
{
  const string s("hello world");
  CHECK(s.find("wo", 0, 2) == 6);
  CHECK(s.find("wor", 0, 3) == 6);
  CHECK(s.find("world", 0, 5) == 6);
}

TEST_CASE("find 3-arg not found")
{
  const string s("hello world");
  CHECK(s.find("xyz", 0, 3) == string::npos);
}

TEST_CASE("find 3-arg with embedded null")
{
  const char needle[] = "he\0llo";
  const string haystack("he\0llo world", 12);
  CHECK(haystack.find(needle, 0, 6) == 0);
}

TEST_CASE("find 3-arg with pos beyond size")
{
  const string s("hello");
  CHECK(s.find("lo", 100, 2) == string::npos);
}

TEST_CASE("find 3-arg zero count")
{
  const string s("hello");
  CHECK(s.find("anything", 2, 0) == 2);
}

TEST_CASE("rfind 3-arg")
{
  const string s("hello world world");
  CHECK(s.rfind("world", 17, 5) == 12);
  CHECK(s.rfind("world", 11, 5) == 6);
  CHECK(s.rfind("xyz", 17, 3) == string::npos);
}

TEST_CASE("find_first_of 3-arg")
{
  const string s("hello world");
  CHECK(s.find_first_of("xyzwo", 0, 5) == 4);
  CHECK(s.find_first_of("xyz", 0, 3) == string::npos);
}

TEST_CASE("find_last_of 3-arg")
{
  const string s("hello world");
  CHECK(s.find_last_of("lo", 17, 2) == 9);
  CHECK(s.find_last_of("xz", 17, 2) == string::npos);
}

TEST_CASE("find_first_not_of 3-arg")
{
  const string s("aabbc");
  CHECK(s.find_first_not_of("ab", 0, 2) == 4);
  CHECK(s.find_first_not_of("abc", 0, 3) == string::npos);
}

TEST_CASE("find_last_not_of 3-arg")
{
  const string s("bcaaa");
  CHECK(s.find_last_not_of("a", 17, 1) == 1);
  CHECK(s.find_last_not_of("abc", 17, 3) == string::npos);
}

TEST_CASE("replace counted overload")
{
  string s("heXXXorld");
  s.replace(2, 3, "llo w", 5);
  CHECK(strcmp(s.c_str(), "hello world") == 0);
}

TEST_CASE("replace counted with same length")
{
  string s("abc");
  s.replace(0, 3, "xyz", 3);
  CHECK(strcmp(s.c_str(), "xyz") == 0);
}

TEST_CASE("replace counted empty replacement")
{
  string s("hello");
  s.replace(0, 5, "", 0);
  CHECK(s.empty());
}

TEST_CASE("replace counted with pos beyond end")
{
  string s("hello");
  s.replace(100, 5, "!!", 2);
  CHECK(strcmp(s.c_str(), "hello!!") == 0);
}

TEST_CASE("replace counted with count exceeding size")
{
  string s("hello");
  s.replace(2, 100, "xyz", 3);
  CHECK(strcmp(s.c_str(), "hexyz") == 0);
}
