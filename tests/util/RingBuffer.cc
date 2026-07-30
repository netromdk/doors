#include <kernel/RingBuffer.h>

#include <doctest/doctest.h>

namespace {

constexpr size_t CAPACITY = 4;
using IntRing = RingBuffer<int, CAPACITY>;

} // namespace

TEST_CASE("empty")
{
  const IntRing ring;
  int out{};
  CHECK(ring.latest(out) == false);
  CHECK(ring.size() == 0);
}

TEST_CASE("single_push")
{
  IntRing ring;
  ring.push(42);
  int out{};
  CHECK(ring.latest(out) == true);
  CHECK(out == 42);
  CHECK(ring.size() == 1);
}

TEST_CASE("multiple_pushes")
{
  IntRing ring;
  for (int i = 1; i <= 3; ++i) {
    ring.push(i * 10);
    int out{};
    CHECK(ring.latest(out) == true);
    CHECK(out == i * 10);
  }
  CHECK(ring.size() == 3);
}

TEST_CASE("wrap_overwrite")
{
  IntRing ring;
  for (size_t i = 0; i < CAPACITY + 3; ++i) {
    ring.push(static_cast<int>(i));
  }
  CHECK(ring.size() == CAPACITY);
  int out{};
  CHECK(ring.latest(out) == true);
  CHECK(out == static_cast<int>(CAPACITY + 2));
}

TEST_CASE("wrap_size_accurate")
{
  IntRing ring;
  for (size_t i = 0; i < CAPACITY + 5; ++i) {
    ring.push(static_cast<int>(i));
    const auto expected = (i < CAPACITY) ? i + 1 : CAPACITY;
    CHECK(ring.size() == expected);
  }
}

TEST_CASE("latest_matches_last_push")
{
  IntRing ring;
  for (size_t i = 0; i < CAPACITY * 2; ++i) {
    ring.push(static_cast<int>(i));
    int out{};
    CHECK(ring.latest(out) == true);
    CHECK(out == static_cast<int>(i));
  }
}
