#ifndef cetlib_simultaneous_function_spawner_h
#define cetlib_simultaneous_function_spawner_h

//=================================================================
// The 'simultaneous_function_spawner' is intended for testing the
// behavior under simultaneous execution of a supplied functor.  The
// functor can receive NO ARGUMENTS.  This class should be used in the
// following pattern (e.g.):
//
// Using repeated tasks:
//
//   std::atomic<unsigned> nTimesCalled {0u};
//   auto count = [&counter]{ ++counter; };
//   // Create 7 tasks of the function 'count'.
//   using namespace hep::concurrency;
//   simultaneous_function_spawner sps{repeated_task(7u, count)};
//   std::cout << nTimesCalled <<'\n'; // Will print out '7'.
//
// Using separate tasks
//
//   std::vector<std::function<void()>> tasks;
//   std::vector<int> nums (2);
//   tasks.push_back([&nums]{ nums[0] = 93; });
//   tasks.push_back([&nums]{ nums[1] = 23; });
//   hep::concurrency::simultaneous_function_spawner sps {tasks};
//
// The number of threads spawned is equan to the number of tasks
// supplied.
//
// The waiting mechanism here could be replaced with a
// condition_variable, similar to what Chris Green implemented in
// wirecap-libs/Barrier.hpp.
// =================================================================

#include <atomic>
#include <thread>
#include <vector>

namespace hep::concurrency {

  template <typename F>
  auto
  repeated_task(std::size_t const nInstances, F func)
  {
    return std::vector<F>(nInstances, func);
  }

  class simultaneous_function_spawner {
  public:
    template <typename function_to_spawn>
    simultaneous_function_spawner(std::vector<function_to_spawn> const& fs)
      : counter_{fs.size()}
    {
      auto execute = [this](auto f) {
        --counter_;
        while (counter_ != 0)
          ;
        f();
      };

      std::vector<std::thread> threads;
      for (auto f : fs) {
        threads.emplace_back(execute, f);
      }

      for (auto& thread : threads)
        thread.join();
    }

  private:
    std::atomic<std::size_t> counter_;
  };
}

#endif /* cetlib_simultaneous_function_spawner_h */

// Local variables:
// mode: c++
// End:
