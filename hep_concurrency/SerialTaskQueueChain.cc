// vim: set sw=2 expandtab :
#include "hep_concurrency/SerialTaskQueueChain.h"

#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/SerialTaskQueue.h"

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

using namespace std;

namespace hep::concurrency {

  SerialTaskQueueChain::SerialTaskQueueChain(
    vector<shared_ptr<SerialTaskQueue>> queues)
    : queues_{move(queues)}
  {}

} // namespace hep::concurrency
