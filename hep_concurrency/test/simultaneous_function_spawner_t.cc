#include "hep_concurrency/simultaneous_function_spawner.h"

#include <catch2/catch.hpp>
#include <iostream>

using namespace hep::concurrency;

TEST_CASE("cout")
{
  auto task = [] { std::cout << "Hello concurrent world.\n"; };
  simultaneous_function_spawner sfs{repeated_task(7u, task)};
}

TEST_CASE("atomic int")
{
  std::atomic<int> i{1};
  auto task = [&i] { ++i; };
  simultaneous_function_spawner sfs{repeated_task(7u, task)};
  CHECK(i == 8);
}

TEST_CASE("assign different numbers")
{
  std::vector<int> nums(7); // All 7 numbers initialized to 0.
  std::vector<std::function<void()>> tasks;
  tasks.push_back([&nums] { nums[0] = 1; });
  tasks.push_back([&nums] { nums[1] = 3; });
  tasks.push_back([&nums] { nums[2] = 5; });
  tasks.push_back([&nums] { nums[3] = 7; });
  tasks.push_back([&nums] { nums[4] = 6; });
  tasks.push_back([&nums] { nums[5] = 4; });
  tasks.push_back([&nums] { nums[6] = 2; });
  auto const ref = std::vector{1, 3, 5, 7, 6, 4, 2};
  simultaneous_function_spawner sfs{tasks};
  CHECK(nums == ref);
}
