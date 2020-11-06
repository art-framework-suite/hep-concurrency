#include "hep_concurrency/assert_only_one_thread.h"
#include "hep_concurrency/simultaneous_function_spawner.h"

#include <chrono>
#include <thread>

using namespace hep::concurrency;
using namespace std::chrono_literals;

// This test is 'fragile' in that, on a heavily loaded system, we
// might see:
//    1. all threads hit the "barrier" in SimultaneousFunctionSpawner
//    2. all threads start executing
//    3. one thread swaps out before getting to increment the counter
//        in the ThreadCounter.
//    4. the other thread completes (increments and then decrements
//        the counter)
//    5. the swapped thread comes back, and is not running at the
//        same time as the other thread, so the expected violation
//        is not seen.

// We attempt to make this very rare by:
//    running a function that takes a long time, and
//    running many threads, hoping not all will swap out in
//    the right order so that we see NO overlaps.
int
main()
{
  HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD();
  auto will_fail = [] {
    // Insert "by-hand" what the
    // HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD would expand to so that
    // we can avoid an std::abort() for testing.
    static detail::thread_counter s{__FILE__, __LINE__, __func__};
    detail::thread_counter::sentry hold{s, false};
    std::this_thread::sleep_for(100ms);
  };
  simultaneous_function_spawner launch{repeated_task(10u, will_fail)};
}
