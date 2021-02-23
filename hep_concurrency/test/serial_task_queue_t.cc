// Test cases originally formed by Chris Jones (9/27/11), who provided
// the initial implementation of the SerialTaskQueue.  They have been
// subsequently adjusted to support TBB's changing interface.

#include <catch2/catch.hpp>

// Catch2's macros cannot generally be invoked concurrently from multiple
// threads.  However, because the SerialTaskQueue serializes access to
// each task, we may make use of the macros here.

#include "hep_concurrency/SerialTaskQueue.h"

#include <atomic>
#include <thread>

TEST_CASE("Push entries")
{
  tbb::task_group group;
  hep::concurrency::SerialTaskQueue queue{group};

  auto count = 0u;
  constexpr auto expected = 1000u;
  for (unsigned int i = 0u; i != expected; ++i) {
    queue.push([&count, i] { CHECK(count++ == i); });
  }
  group.wait();
  CHECK(count == expected);
}

TEST_CASE("Pause execution")
{
  tbb::task_group group;
  hep::concurrency::SerialTaskQueue queue{group};
  queue.pause();

  auto count = 0u;
  constexpr auto expected = 100u;
  for (std::size_t i = 0; i != expected; ++i) {
    queue.push([&count] { ++count; });
  }
  group.wait();
  CHECK(count == 0u);

  queue.resume();
  group.wait();
  CHECK(count == expected);
}

TEST_CASE("Stress the queue from multiple threads")
{
  tbb::task_group group;
  hep::concurrency::SerialTaskQueue queue{group};
  constexpr auto n_tasks = 1000u;
  for (unsigned int j = 0u; j != 100u; ++j) {
    std::atomic count{0u};
    auto push_tasks = [&queue, &count] {
      for (unsigned int i = 0; i != n_tasks; ++i) {
        queue.push([&count] { ++count; });
      }
    };

    std::thread push_thread{push_tasks};
    push_tasks();
    push_thread.join();
    group.wait();
    CHECK(2 * n_tasks == count);
  }
}
