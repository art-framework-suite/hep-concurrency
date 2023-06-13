#include "hep_concurrency/SerialTaskQueueChain.h"

// generates a random number
unsigned
foo(unsigned bar)
{
  bar = 0xACE1u;
  unsigned bit;
  bit = ((bar >> 0) ^ (bar >> 2) ^ (bar >> 3) ^ (bar >> 5)) & 1;
  return bar = (bar >> 1) | (bit << 15);
}

// generates queue chains and then passes foo(bar) into them. should fail.
int
main()
{
  tbb::task_group group;
  std::vector queues{
    std::make_shared<hep::concurrency::SerialTaskQueue>(group),
    std::make_shared<hep::concurrency::SerialTaskQueue>(group)};
  hep::concurrency::SerialTaskQueueChain chain{queues};

  chain.push(foo(5));
  return 0;
}
