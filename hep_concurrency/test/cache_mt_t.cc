#include <catch2/catch.hpp>

#include "hep_concurrency/cache.h"
#include "hep_concurrency/test/interval_of_validity.h"

#include "tbb/parallel_for_each.h"

#include <algorithm>
#include <random>
#include <string>
#include <utility>
#include <vector>

using hep::concurrency::cache;
using hep::concurrency::test::interval_of_validity;

namespace {

  constexpr auto num_events = 20;
  constexpr auto half_of_them = num_events / 2;

  std::vector<std::pair<interval_of_validity, std::string>> iovs{
    {interval_of_validity{0, half_of_them}, "Good"},
    {interval_of_validity{half_of_them, num_events}, "Bad"}};

  class CalibrationQuality {
  public:
    auto
    quality_for(unsigned int const event)
    {
      if (auto h = calibration_quality_.entry_for(event)) {
        return h;
      }

      for (auto const& [iov, quality] : iovs) {
        if (iov.supports(event)) {
          return calibration_quality_.emplace(iov, quality);
        }
      }

      throw cet::exception("Data not found");
    }

    void
    drop_unused(unsigned const n = 0)
    {
      calibration_quality_.drop_unused_but_last(n);
    }

  private:
    cache<interval_of_validity, std::string> calibration_quality_;
  };

  auto
  event_numbers()
  {
    std::vector<unsigned> result(num_events);
    auto b = begin(result);
    auto e = end(result);
    std::iota(b, e, 0);
    std::shuffle(b, e, std::mt19937{std::random_device{}()});
    return result;
  }

  class value_counter {
  public:
    void
    tally(unsigned const event, std::string const& value)
    {
      if (event < half_of_them and value == "Good") {
        ++the_goods_;
      } else if (event >= half_of_them and value == "Bad") {
        ++the_bads_;
      } else {
        ++the_uglies_;
      }
    }

    bool
    correct_tally() const
    {
      auto const success = the_goods_ == half_of_them and
                           the_bads_ == half_of_them and the_uglies_ == 0u;
      if (not success)
        std::cout << "Uh oh: " << the_goods_ << ", " << the_bads_ << ", "
                  << the_uglies_ << '\n';
      return success;
    }

  private:
    std::atomic<unsigned> the_goods_{};
    std::atomic<unsigned> the_bads_{};
    std::atomic<unsigned> the_uglies_{};
  };

  class count_data {
  public:
    count_data(value_counter& counter, unsigned const drop_n = -1u)
      : counter_{counter}, n_{drop_n}
    {}

    // This is the function that is potentially called from multiple threads.
    void
    operator()(unsigned const event) const
    {
      auto h = calibration_->quality_for(event);
      counter_.tally(event, *h);
      if (n_ == -1u)
        return;
      calibration_->drop_unused(n_);
    }

  private:
    std::shared_ptr<CalibrationQuality> calibration_{
      std::make_shared<CalibrationQuality>()};
    value_counter& counter_;
    unsigned n_;
  };

}

TEST_CASE("User-defined (single-threaded)")
{
  auto const events = event_numbers();
  auto const b = begin(events);
  auto const e = end(events);
  value_counter counter;

  SECTION("Drop nothing")
  {
    std::for_each(b, e, count_data{counter});
    CHECK(counter.correct_tally());
  }
  SECTION("Drop all unused")
  {
    std::for_each(b, e, count_data{counter, 0});
    CHECK(counter.correct_tally());
  }
  SECTION("Drop all but 1 unused")
  {
    std::for_each(b, e, count_data{counter, 1});
    CHECK(counter.correct_tally());
  }
  SECTION("Drop all but 2 unused")
  {
    std::for_each(b, e, count_data{counter, 2});
    CHECK(counter.correct_tally());
  }
}

TEST_CASE("User-defined (multi-threaded)")
{
  auto const events = event_numbers();
  value_counter counter;

  SECTION("Drop nothing")
  {
    tbb::parallel_for_each(events, count_data{counter});
    CHECK(counter.correct_tally());
  }
  SECTION("Drop all unused")
  {
    tbb::parallel_for_each(events, count_data{counter, 0});
    CHECK(counter.correct_tally());
  }
  SECTION("Drop all but 1 unused")
  {
    tbb::parallel_for_each(events, count_data{counter, 1});
    CHECK(counter.correct_tally());
  }
  SECTION("Drop all but 2 unused")
  {
    tbb::parallel_for_each(events, count_data{counter, 2});
    CHECK(counter.correct_tally());
  }
}
