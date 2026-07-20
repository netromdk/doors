#include <doctest/doctest.h>
#include <span>
#include <string>

TEST_CASE("span_default_empty")
{
  const span<int> s;
  CHECK(s.data() == nullptr);
  CHECK(s.size() == 0);
  CHECK(s.empty());
}

TEST_CASE("span_ptr_size_constructor")
{
  int arr[3] = {10, 20, 30};
  const span<int> s(arr, 3);
  CHECK(s.data() == arr);
  CHECK(s.size() == 3);
  CHECK(!s.empty());
}

TEST_CASE("span_index_access")
{
  int arr[3] = {10, 20, 30};
  const span<int> s(arr, 3);
  CHECK(s[0] == 10);
  CHECK(s[1] == 20);
  CHECK(s[2] == 30);
}

TEST_CASE("span_iteration")
{
  int arr[3] = {10, 20, 30};
  const span<int> s(arr, 3);
  int sum = 0;
  for (auto &v : s) {
    sum += v;
  }
  CHECK(sum == 60);
}

TEST_CASE("span_begin_end")
{
  int arr[3] = {10, 20, 30};
  span<int> s(arr, 3);
  CHECK(*s.begin() == 10);
  CHECK(*(s.end() - 1) == 30);
}

TEST_CASE("span_const")
{
  const int arr[2] = {1, 2};
  const span<const int> s(arr, 2);
  CHECK(s[0] == 1);
  CHECK(s[1] == 2);
}

TEST_CASE("span_write_through_pointer")
{
  int val = 42;
  span<int> s(&val, 1);
  s[0] = 99;
  CHECK(s[0] == 99);
}

TEST_CASE("span_empty_from_default")
{
  const span<int> s;
  CHECK(s.begin() == s.end());
}

TEST_CASE("span_string")
{
  string arr[2] = {"hello", "world"};
  span<string> s(arr, 2);
  CHECK(s[0] == "hello");
  CHECK(s[1] == "world");
}
