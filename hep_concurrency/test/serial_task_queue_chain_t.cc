// Test cases originally formed by Chris Jones (9/27/11), who provided
// the initial implementation of the SerialTaskQueueChain.  They have
// been subsequently adjusted to support TBB's changing interface.

#include <catch2/catch_test_macros.hpp>

// Catch2's macros cannot generally be invoked concurrently from
// multiple threads.  However, because the SerialTaskQueueChain
// serializes access to each task, we may make use of the macros here.

#include "hep_concurrency/SerialTaskQueueChain.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

using namespace hep::concurrency;
using namespace std::chrono_literals;

TEST_CASE("Push entries")
{
  std::atomic count = 0u;
  tbb::task_group group;
  std::vector queues{std::make_shared<SerialTaskQueue>(group),
                     std::make_shared<SerialTaskQueue>(group)};
  SerialTaskQueueChain chain{queues};
  auto check_value = [&count](unsigned int i) {
    return [&count, i] {
      CHECK(count++ == i);
      std::this_thread::sleep_for(10us);
    };
  };
  for (unsigned int i = 0; i != 3; ++i) {
    chain.push(check_value(i));
  }
  group.wait();
  CHECK(count == 3u);
}

namespace {
  template <typename T>
  void
  wait_at_most_30_seconds(T predicate, std::string const& err_msg)
  {
    auto now = std::chrono::steady_clock::now;
    auto begin = now();
    while (now() - begin < 30s) {
      if (predicate()) {
        return;
      }
    }
    throw std::logic_error(err_msg);
  }
}

TEST_CASE("Stress the chain from multiple threads")
{
  tbb::task_group group;
  std::vector queues{std::make_shared<SerialTaskQueue>(group),
                     std::make_shared<SerialTaskQueue>(group)};
  SerialTaskQueueChain chain{queues};
  std::atomic wait_to_start{true};
  std::atomic count_stress_tasks{0u};
  auto count_stress = 0u;
  constexpr auto nTasks = 1000u;
  constexpr auto expected_count = 5 * nTasks;

  auto launch_stress_tasks = [&chain, &count_stress] {
    for (unsigned i = 0u; i != nTasks; ++i) {
      chain.push([&count_stress] { ++count_stress; });
    }
  };

  auto stressFunctor =
    [&wait_to_start, &count_stress_tasks, &launch_stress_tasks] {
      ++count_stress_tasks;
      wait_at_most_30_seconds(
        [&] { return not wait_to_start.load(); },
        "Max wait time exceeded! wait_to_start never became false!");
      launch_stress_tasks();
    };

  for (unsigned index = 0u; index != 10u; ++index) {
    wait_to_start = true;
    count_stress_tasks = 0u;
    count_stress = 0u;

    // Launch tasks from STL thread
    std::thread the_thread{stressFunctor};

    // Launch via TBB's task group
    for (int i = 0; i != 3; ++i) {
      group.run(stressFunctor);
    }

    // Allow stress tasks and the thread to proceed
    wait_to_start = false;

    group.wait();

    wait_at_most_30_seconds(
      [&] { return count_stress_tasks == 4u; },
      "Max counting wait exceeded! count_stress_tasks never reached 4!");

    // Launch tasks on main thread
    launch_stress_tasks();

    wait_at_most_30_seconds([&] { return count_stress == expected_count; },
                            "Max counting wait exceeded! count_stress never "
                            "reached expected_count!");

    group.wait();
    the_thread.join();
  }
}
