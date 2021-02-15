#include "hep_concurrency/SerialTaskQueue.h"

#include "tbb/task_group.h"

#include <cassert>

int
main()
{
  tbb::task_group group;
  hep::concurrency::SerialTaskQueue queue{group};
  int sum = 0;
  auto add_one = [&sum] { ++sum; };

  constexpr int expected = 1'000'000;

  for (int i = 0; i != expected; ++i) {
    queue.push(add_one);
  }
  group.wait();
  assert(sum == expected);
}
