#include <doctest/doctest.h>
#include <string_view>

TEST_CASE("default construction")
{
  const string_view sv;
  CHECK(sv.data() == nullptr);
  CHECK(sv.size() == 0);
  CHECK(sv.empty());
}

TEST_CASE("from const char*")
{
  const string_view sv("hello");
  CHECK(sv.size() == 5);
  CHECK(sv.empty() == false);
  CHECK(sv[0] == 'h');
  CHECK(sv[4] == 'o');
}

TEST_CASE("from const char* with count")
{
  const char *s = "hello world";
  const string_view sv(s, 5);
  CHECK(sv.size() == 5);
  CHECK(sv[0] == 'h');
  CHECK(sv[4] == 'o');
}

TEST_CASE("from nullptr const char*")
{
  const char *p = nullptr;
  const string_view sv(p);
  CHECK(sv.size() == 0);
  CHECK(sv.empty());
}

TEST_CASE("data and size")
{
  const char *s = "test";
  const string_view sv(s);
  CHECK(sv.data() == s);
  CHECK(sv.size() == 4);
  CHECK(sv.length() == 4);
}

TEST_CASE("front and back")
{
  const string_view sv("hello");
  CHECK(sv.front() == 'h');
  CHECK(sv.back() == 'o');
}

TEST_CASE("at")
{
  const string_view sv("hello");
  CHECK(sv.at(0) == 'h');
  CHECK(sv.at(4) == 'o');
  CHECK(sv.at(10) == sv.front()); // out of bounds returns first
}

TEST_CASE("iteration")
{
  const string_view sv("abc");
  int count = 0;
  for (auto c : sv) {
    CHECK(c == "abc"[count]);
    ++count;
  }
  CHECK(count == 3);
}

TEST_CASE("remove_prefix")
{
  string_view sv("hello");
  sv.remove_prefix(2);
  CHECK(sv.size() == 3);
  CHECK(sv[0] == 'l');
}

TEST_CASE("remove_suffix")
{
  string_view sv("hello");
  sv.remove_suffix(2);
  CHECK(sv.size() == 3);
  CHECK(sv[2] == 'l');
}

TEST_CASE("copy")
{
  const string_view sv("hello");
  char buf[8] = {};
  auto n = sv.copy(buf, 3, 1);
  CHECK(n == 3);
  CHECK(buf[0] == 'e');
  CHECK(buf[1] == 'l');
  CHECK(buf[2] == 'l');
}

TEST_CASE("substr")
{
  const string_view sv("hello world");
  auto sub = sv.substr(6, 5);
  CHECK(sub.size() == 5);
  CHECK(sub[0] == 'w');
  CHECK(sub[4] == 'd');
}

TEST_CASE("substr past end")
{
  const string_view sv("hi");
  auto sub = sv.substr(10);
  CHECK(sub.empty());
}

TEST_CASE("substr npos")
{
  const string_view sv("hello");
  auto sub = sv.substr(2, string_view::npos);
  CHECK(sub.size() == 3);
  CHECK(sub[0] == 'l');
  CHECK(sub[2] == 'o');
}

TEST_CASE("compare equal")
{
  string_view a("hello"), b("hello");
  CHECK(a.compare(b) == 0);
  CHECK(a == b);
}

TEST_CASE("compare less")
{
  string_view a("apple"), b("banana");
  CHECK(a.compare(b) < 0);
  CHECK(a < b);
}

TEST_CASE("compare greater")
{
  string_view a("banana"), b("apple");
  CHECK(a.compare(b) > 0);
  CHECK(a > b);
}

TEST_CASE("comparison operators")
{
  string_view a("abc"), b("abc"), c("abd"), d("ab");
  CHECK(a == b);
  CHECK(a != c);
  CHECK(a < c);
  CHECK(c > a);
  CHECK(d <= b);
  CHECK(c >= a);
}

TEST_CASE("find")
{
  const string_view sv("hello world");
  CHECK(sv.find("world") == 6);
  CHECK(sv.find("lo") == 3);
  CHECK(sv.find("xyz") == string_view::npos);
}

TEST_CASE("find with pos")
{
  const string_view sv("hello hello");
  CHECK(sv.find("hello", 1) == 6);
}

TEST_CASE("rfind")
{
  const string_view sv("hello hello");
  CHECK(sv.rfind("hello") == 6);
  CHECK(sv.rfind("xyz") == string_view::npos);
}

TEST_CASE("find_first_of")
{
  const string_view sv("hello world");
  CHECK(sv.find_first_of("aeiou") == 1); // 'e'
}

TEST_CASE("find_last_of")
{
  const string_view sv("hello world");
  CHECK(sv.find_last_of("aeiou") == 7); // 'o' in "world"
}

TEST_CASE("find_first_not_of")
{
  const string_view sv("   hello");
  CHECK(sv.find_first_not_of(" ") == 3);
}

TEST_CASE("find_last_not_of")
{
  const string_view sv("hello   ");
  CHECK(sv.find_last_not_of(" ") == 4);
}

TEST_CASE("contains")
{
  const string_view sv("hello world");
  CHECK(sv.contains("world"));
  CHECK(sv.contains("hello"));
  CHECK(!sv.contains("xyz"));
}

TEST_CASE("starts_with")
{
  const string_view sv("hello world");
  CHECK(sv.starts_with("hello"));
  CHECK(!sv.starts_with("world"));
}

TEST_CASE("ends_with")
{
  const string_view sv("hello world");
  CHECK(sv.ends_with("world"));
  CHECK(!sv.ends_with("hello"));
}

TEST_CASE("swap")
{
  string_view a("hello"), b("world");
  swap(a, b);
  CHECK(a == "world");
  CHECK(b == "hello");
}

TEST_CASE("member swap")
{
  string_view a("hello"), b("world");
  a.swap(b);
  CHECK(a == "world");
  CHECK(b == "hello");
}

TEST_CASE("constexpr usage")
{
  constexpr const string_view sv("hello");
  constexpr auto n = sv.size();
  constexpr auto c = sv[1];
  CHECK(n == 5);
  CHECK(c == 'e');
}

TEST_CASE("constexpr comparison")
{
  constexpr string_view a("abc"), b("abc");
  constexpr bool eq = (a == b);
  CHECK(eq);
}

TEST_CASE("empty string_view")
{
  const string_view sv("");
  CHECK(sv.size() == 0);
  CHECK(sv.empty());
  CHECK(sv == string_view());
}
