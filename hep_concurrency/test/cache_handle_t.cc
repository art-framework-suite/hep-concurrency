#include <catch2/catch.hpp>

#include "cetlib_except/exception_message_matcher.h"
#include "hep_concurrency/cache.h"
#include "interval_of_validity.h"

#include <utility>

using Catch::Matchers::Contains;
using namespace hep::concurrency;

TEST_CASE("Invalid handle")
{
  auto h = cache_handle<std::string, int>::invalid();
  CHECK(not h);
  CHECK_THROWS_MATCHES(
    h.key(),
    cet::exception,
    cet::exception_message_matcher(Contains("Invalid key access.")));
  CHECK_THROWS_MATCHES(h.sequence_number(),
                       cet::exception,
                       cet::exception_message_matcher(
                         Contains("Invalid sequence-number access.")));
  CHECK_THROWS_MATCHES(*h,
                       cet::exception,
                       cet::exception_message_matcher(
                         Contains("Invalid cache handle dereference.")));
}

TEST_CASE("One entry")
{
  cache<std::string, int> office_numbers;
  auto h = office_numbers.emplace("Alice", 97);
  REQUIRE(h);
  CHECK(h.key() == "Alice");
  CHECK(h.sequence_number() == 0ull);
  CHECK(*h == 97);
}

TEST_CASE("Invalidate handle")
{
  cache<std::string, int> office_numbers;
  auto h = office_numbers.emplace("Alice", 97);
  h.invalidate();
  CHECK_THROWS_MATCHES(
    h.key(),
    cet::exception,
    cet::exception_message_matcher(Contains("Invalid key access.")));
  CHECK_THROWS_MATCHES(h.sequence_number(),
                       cet::exception,
                       cet::exception_message_matcher(
                         Contains("Invalid sequence-number access.")));
  CHECK_THROWS_MATCHES(*h,
                       cet::exception,
                       cet::exception_message_matcher(
                         Contains("Invalid cache handle dereference.")));
}

TEST_CASE("Copy-construct handle")
{
  cache<std::string, int> office_numbers;
  auto tmp_h = office_numbers.emplace("Alice", 97);
  auto h = tmp_h;
  CHECK(tmp_h); // tmp_h still valid
  REQUIRE(h);
  CHECK(h.key() == "Alice");
  CHECK(h.sequence_number() == 0ull);
  CHECK(*h == 97);
}

TEST_CASE("Move-construct handle")
{
  cache<std::string, int> office_numbers;
  auto tmp_h = office_numbers.emplace("Alice", 97);
  auto h = std::move(tmp_h);
  CHECK(not tmp_h); // tmp_h no longer valid
  REQUIRE(h);
  CHECK(h.key() == "Alice");
  CHECK(h.sequence_number() == 0ull);
  CHECK(*h == 97);
}

TEST_CASE("Copy-assign handle")
{
  cache<std::string, int> office_numbers;
  auto tmp_h = office_numbers.emplace("Alice", 97);
  auto h = decltype(tmp_h)::invalid();
  h = tmp_h;
  CHECK(tmp_h); // tmp_h still valid
  REQUIRE(h);
  CHECK(h.key() == "Alice");
  CHECK(h.sequence_number() == 0ull);
  CHECK(*h == 97);
}

TEST_CASE("Move-assign handle")
{
  cache<std::string, int> office_numbers;
  auto tmp_h = office_numbers.emplace("Alice", 97);
  auto h = decltype(tmp_h)::invalid();
  h = std::move(tmp_h);
  CHECK(not tmp_h); // tmp_h no longer valid
  REQUIRE(h);
  CHECK(h.key() == "Alice");
  CHECK(h.sequence_number() == 0ull);
  CHECK(*h == 97);
}

TEST_CASE("Handle comparisons")
{
  using cache_t = cache<std::string, int>;
  using handle_t = cache_t::handle;

  cache_t office_numbers;
  office_numbers.emplace("Alice", 97);
  office_numbers.emplace("David", 98);

  SECTION("Both invalid keys")
  {
    auto const h1 = handle_t::invalid();
    auto const h2 = handle_t::invalid();
    CHECK(h1 == h1);
    CHECK(h1 == h2);
  }
  SECTION("Invalid and valid keys")
  {
    auto const invalid = handle_t::invalid();
    auto h = office_numbers.at("Alice");
    CHECK(invalid != h);
    h.invalidate();
    CHECK(invalid == h);
  }
  SECTION("Both valid keys")
  {
    auto const alice1 = office_numbers.at("Alice");
    auto const alice2 = alice1;
    CHECK(alice1);
    CHECK(alice2);
    CHECK(alice1 == alice2);
    auto const david = office_numbers.at("David");
    CHECK(david);
    CHECK(david != alice1);
  }
}

TEST_CASE("User-defined key")
{
  cache<test::interval_of_validity, std::string> cache;
  auto h = cache.emplace({1, 10}, "Run 1");
  CHECK(h.key().supports(2));
}

TEST_CASE("Sequence number changes")
{
  cache<test::interval_of_validity, std::string> cache;
  CHECK(cache.emplace({1, 10}, "Run 1").sequence_number() == 0ull);
  cache.drop_unused();
  CHECK(cache.emplace({1, 10}, "Run 1").sequence_number() == 1ull);
}
