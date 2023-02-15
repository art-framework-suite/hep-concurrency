// Test cases originally formed by Chris Jones (9/27/11), who provided
// the initial implementation of the WaitingTaskList.  They have been
// subsequently adjusted to support TBB's changing interface.

#include <catch2/catch_test_macros.hpp>

#include "hep_concurrency/WaitingTaskList.h"
#include "tbb/task_group.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <thread>

using namespace hep::concurrency;
using namespace std::chrono_literals;
using namespace std::string_literals;

namespace {
  class TestTask {
  public:
    TestTask(std::atomic<unsigned>& call_count, std::exception_ptr& ex_ptr)
      : call_count_{call_count}, ex_{ex_ptr}
    {}

    void
    operator()(std::exception_ptr ex_ptr)
    {
      if (ex_ptr) {
        ex_ = ex_ptr;
      }
      ++call_count_;
    }

  private:
    std::atomic<unsigned>& call_count_;
    std::exception_ptr& ex_;
  };
}

TEST_CASE("Add task then declare done waiting")
{
  tbb::task_group group;
  WaitingTaskList wait_list{group};

  std::atomic<unsigned> call_count{};
  std::exception_ptr ex_ptr{};
  wait_list.add(make_waiting_task<TestTask>(call_count, ex_ptr));
  std::this_thread::sleep_for(10us);
  group.wait();

  // doneWaiting not yet called, so task cannot have run.
  CHECK(call_count == 0u);

  wait_list.doneWaiting();
  group.wait();

  CHECK(call_count == 1u);
  CHECK(not ex_ptr);
}

TEST_CASE("Declare done waiting and then add task")
{
  tbb::task_group group;
  WaitingTaskList wait_list{group};
  wait_list.doneWaiting();

  // Done waiting has been declared, so any tasks added will be
  // immediately run.
  std::atomic<unsigned> call_count{};
  std::exception_ptr ex_ptr{};
  wait_list.add(make_waiting_task<TestTask>(call_count, ex_ptr));
  group.wait();

  CHECK(call_count == 1u);
  CHECK(not ex_ptr);
}

TEST_CASE("Add task then declare done waiting with exception")
{
  tbb::task_group group;
  WaitingTaskList wait_list{group};

  std::atomic<unsigned> call_count{};
  std::exception_ptr ex_ptr;
  wait_list.add(make_waiting_task<TestTask>(call_count, ex_ptr));
  group.wait();
  CHECK(call_count == 0u);

  wait_list.doneWaiting(make_exception_ptr("failed"s));
  std::this_thread::sleep_for(10us);
  group.wait();

  CHECK(call_count == 1u);
  CHECK(ex_ptr);
}

TEST_CASE("Declare done waiting with exception and then add task")
{
  tbb::task_group group;
  WaitingTaskList wait_list{group};
  wait_list.doneWaiting(make_exception_ptr("failed"s));

  // Done waiting has been declared, so any tasks added will be
  // immediately run.
  std::atomic<unsigned> call_count{};
  std::exception_ptr ex_ptr{};
  wait_list.add(make_waiting_task<TestTask>(call_count, ex_ptr));
  group.wait();

  CHECK(call_count == 1u);
  CHECK(ex_ptr);
}

TEST_CASE("Stress the waiting list from multiple threads")
{
  tbb::task_group group;
  hep::concurrency::WaitingTaskList wait_list{group};

  constexpr auto n_tasks = 100u;
  for (unsigned int index = 0; index != 10; ++index) {
    std::atomic call_count{0u};
    std::exception_ptr ex_ptr{};
    std::thread makeTasksThread([&wait_list, &call_count, &ex_ptr] {
      for (unsigned int i = 0; i < n_tasks; ++i) {
        wait_list.add(make_waiting_task<TestTask>(call_count, ex_ptr));
      }
    });
    group.wait();
    makeTasksThread.join();
    CHECK(call_count == 0u);

    // Calling doneWaiting will run all tasks.
    std::thread doneWaitThread([&wait_list, &group] {
      wait_list.doneWaiting();
      group.wait();
    });
    doneWaitThread.join();

    CHECK(call_count == n_tasks);

    wait_list.reset();
  }
}
