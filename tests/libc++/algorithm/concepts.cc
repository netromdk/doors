#include <algorithm.h>
#include <doctest/doctest.h>

// InputIterator
static_assert(InputIterator<int *>);
static_assert(InputIterator<const int *>);
static_assert(InputIterator<const char *>);
static_assert(!InputIterator<int>);
static_assert(!InputIterator<void>);

// ForwardIterator
static_assert(ForwardIterator<int *>);
static_assert(ForwardIterator<const int *>);
static_assert(!ForwardIterator<int>);

// Predicate
struct IsPositive {
  bool operator()(int x) const
  {
    return x > 0;
  }
};

struct VoidCallable {
  void operator()(int) const
  {
  }
};

struct NonCallable {};

static_assert(Predicate<IsPositive, int>);
static_assert(Predicate<decltype([](int x) { return x > 0; }), int>);
static_assert(!Predicate<IsPositive, const char *>);
static_assert(!Predicate<NonCallable, int>);

TEST_CASE("find with int pointer satisfies InputIterator")
{
  const int arr[] = {1, 2, 3, 4, 5};
  const auto it = find(arr, arr + 5, 3);
  CHECK(*it == 3);
}

TEST_CASE("find_if with lambda satisfies Predicate")
{
  const int arr[] = {1, 2, 3, 4, 5};
  const auto it = find_if(arr, arr + 5, [](int x) { return x > 3; });
  CHECK(*it == 4);
}

TEST_CASE("all_of with lambda")
{
  const int arr[] = {2, 4, 6, 8};
  CHECK(all_of(arr, arr + 4, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("any_of with lambda")
{
  const int arr[] = {1, 3, 5, 6, 7};
  CHECK(any_of(arr, arr + 5, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("none_of with lambda")
{
  const int arr[] = {1, 3, 5, 7};
  CHECK(none_of(arr, arr + 4, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("count_if with lambda")
{
  const int arr[] = {1, 2, 3, 4, 5, 6};
  CHECK(count_if(arr, arr + 6, [](int x) { return x % 2 == 0; }) == 3);
}

TEST_CASE("fill with pointer satisfies ForwardIterator")
{
  int arr[5] = {1, 2, 3, 4, 5};
  fill(arr, arr + 5, 99);
  CHECK(arr[0] == 99);
  CHECK(arr[4] == 99);
}

TEST_CASE("fill_n with pointer")
{
  int arr[5] = {};
  const auto result = fill_n(arr, 5, 42);
  CHECK(result == arr + 5);
  CHECK(arr[0] == 42);
}

TEST_CASE("copy with pointers satisfies InputIterator")
{
  const int src[3] = {1, 2, 3};
  int dst[3] = {};
  copy(src, src + 3, dst);
  CHECK(dst[0] == 1);
  CHECK(dst[2] == 3);
}

TEST_CASE("copy_n with pointers")
{
  const int src[3] = {10, 20, 30};
  int dst[3] = {};
  copy_n(src, 3, dst);
  CHECK(dst[1] == 20);
}

TEST_CASE("swap_ranges with pointers satisfies ForwardIterator")
{
  int a[2] = {1, 2};
  int b[2] = {3, 4};
  swap_ranges(a, a + 2, b);
  CHECK(a[0] == 3);
  CHECK(b[0] == 1);
}
