#include <catch2/catch_test_macros.hpp>

#include "cetlib_except/exception_message_matcher.h"
#include "hep_concurrency/cache.h"
#include "hep_concurrency/cache_handle.h"
#include "interval_of_validity.h"

#include <string>

using namespace hep::concurrency;

TEST_CASE("simple")
{
  cache<std::string, int> cache;
  CHECK(empty(cache));
  {
    auto h = cache.at("Alice");
    CHECK(not h);
    using Catch::Matchers::ContainsSubstring;
    CHECK_THROWS_MATCHES(*h,
                         cet::exception,
                         cet::exception_message_matcher(ContainsSubstring(
                           "Invalid cache handle dereference.")));
  }
  cache.emplace("Alice", 97);
  CHECK(size(cache) == 1ull);
  {
    auto h = cache.at("Alice");
    CHECK(h);
    CHECK(*h == 97);
  }
  cache.drop_unused_but_last(1);
  CHECK(size(cache) == 1ull);

  cache.drop_unused();
  CHECK(empty(cache));
}

TEST_CASE("Multiple entries")
{
  cache<std::string, int> cache;
  {
    auto h = cache.emplace("Billy", 14);
    CHECK(size(cache) == 1ull);

    cache.drop_unused_but_last(1);
    CHECK(size(cache) == 1ull);

    cache.emplace("Bessie", 19);
    cache.emplace("Jason", 20);
    h = cache.at("Jason");
    CHECK(h);
    CHECK(*h == 20);
    CHECK(size(cache) == 3ull);
  }
  cache.drop_unused_but_last(1);
  CHECK(not cache.at("Billy"));
  CHECK(not cache.at("Bessie"));
  CHECK(size(cache) == 1ull);
}

TEST_CASE("Copied handle")
{
  cache<std::string, int> ages;
  auto h = decltype(ages)::handle::invalid();
  std::size_t bobs_sequence_number{-1ull};
  CHECK(not h);
  {
    auto tmp_h = ages.emplace("Bob", 41);
    bobs_sequence_number = tmp_h.sequence_number();
    h = tmp_h;
  }
  ages.drop_unused();
  CHECK(size(ages) == 1ull);
  CHECK(bobs_sequence_number == h.sequence_number());
  h.invalidate();
  ages.drop_unused();
  CHECK(empty(ages));
}

TEST_CASE("Copy same handle and then drop unused")
{
  cache<std::string, int> ages;
  auto tmp_h = ages.emplace("Catherine", 8);
  auto h{tmp_h};
  tmp_h.invalidate();
  for (unsigned int i{}; i != 3; ++i) {
    h = ages.at("Catherine");
  }
  CHECK(size(ages) == 1ull);
  ages.drop_unused();
  CHECK(size(ages) == 1ull);
  h.invalidate();
  ages.drop_unused();
  CHECK(empty(ages));
}

TEST_CASE("User defined")
{
  cache<test::interval_of_validity, std::string> cache;
  auto const run_1 = "Run 1";
  auto const run_2 = "Run 2";

  auto h = cache.emplace({1, 10}, run_1);
  CHECK(*h == run_1);
  h = cache.emplace({10, 20}, run_2);
  CHECK(*h == run_2);
  h.invalidate();
  CHECK(not cache.entry_for(0));
  SECTION("Verify supports mechanism - different type than key")
  {
    auto const h = cache.entry_for(1);
    CHECK(*h == run_1);
    CHECK(h == cache.entry_for(h, 1)); // Test hint form
    CHECK(*cache.entry_for(10) == run_2);
    CHECK(not cache.entry_for(20));
  }
  SECTION("Verify supports mechanism - same type as key")
  {
    auto const sub_iov = test::interval_of_validity{2, 10};
    auto const h = cache.entry_for(sub_iov);
    CHECK(h.key() == (test::interval_of_validity{1, 10}));
    CHECK(*h == run_1);
    CHECK(h == cache.entry_for(h, sub_iov)); // Test hint form
  }
  cache.drop_unused_but_last(1);
  CHECK(size(cache) == 1ull);
  CHECK(cache.entry_for(10));

  CHECK(cache.capacity() == 2ull);
  cache.shrink_to_fit();
  CHECK(cache.capacity() == 0ull);
  CHECK(empty(cache));
}

TEST_CASE("User defined with hint")
{
  cache<test::interval_of_validity, std::string> cache;
  auto const run_1 = "Run 1";
  auto const run_2 = "Run 2";
  cache.emplace({0, 10}, run_1);
  cache.emplace({10, 20}, run_2);

  unsigned int run_1_count{};
  unsigned int run_2_count{};
  auto cached_handle = decltype(cache)::handle::invalid();

  for (int i{}; i != 20; ++i) {
    // Always used the hint form
    auto h = cache.entry_for(cached_handle, i);
    if (not h) {
      continue;
    }

    if (cached_handle != h) {
      cached_handle = h;
      // Do a lot of work
    }

    if (i < 10) {
      ++run_1_count;
    } else {
      ++run_2_count;
    }
  }

  CHECK(run_1_count == 10u);
  CHECK(run_2_count == 10u);

  cache.shrink_to_fit();
  CHECK(size(cache) == 1u);
  CHECK(*cached_handle == run_2);

  cached_handle.invalidate();
  cache.shrink_to_fit();

  CHECK(cache.capacity() == size(cache));
  CHECK(size(cache) == 0ull);
}

namespace {
  template <typename Key, typename Value>
  concept can_cache = requires { { cache<Key, Value>{} }; };

  struct not_hashable {
    bool
    operator==(not_hashable const& other) const
    {
      return data == other.data;
    }
    int data{3};
  };

  class no_eq {
  private:
    int data;
  };

}

TEST_CASE("Cache constraint enforcement")
{
  CHECK_FALSE(can_cache<not_hashable, int>);
  CHECK_FALSE(can_cache<no_eq, int>);
}

namespace {
  struct test_key {
    int num;

    std::size_t
    hash() const
    {
      return std::hash<int>{}(num);
    }

    bool
    operator==(test_key const& other) const
    {
      return num == other.num;
    }
  };

  template <typename Key, typename Value, typename T>
  concept can_call_entry_for =
  requires (cache<Key, Value> test_cache, T t)
  { { test_cache.entry_for(t) }; };

  template <typename Key, typename Value, typename T>
  concept can_call_entry_for_hint =
  requires (cache<Key, Value> test_cache, T t)
  { { test_cache.entry_for(cache_handle<Key, Value>::invalid(), t) }; };
}

TEST_CASE("entry_for() constraint enforcement (bad)")
{
  // Prevent false passes for CHECK_FALSE() tests.
  REQUIRE(can_cache<test_key, int>);
  CHECK_FALSE(can_call_entry_for<test_key, int, int>);
  CHECK_FALSE(can_call_entry_for_hint<test_key, int, int>);
}

TEST_CASE("entry_for() constraint enforcement (good)")
{
  // Prevent false passes for CHECK_FALSE() tests.
  REQUIRE(can_cache<test::interval_of_validity, std::string>);
  CHECK(can_call_entry_for<test::interval_of_validity, std::string, int>);
  CHECK(can_call_entry_for_hint<test::interval_of_validity, std::string, int>);
  CHECK_FALSE(can_call_entry_for<test::interval_of_validity, std::string, std::string>);
  CHECK_FALSE(can_call_entry_for_hint<test::interval_of_validity, std::string, std::string>);
}
