// vim: set sw=2 expandtab :
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <memory>
#include <vector>

using namespace std;

namespace hep::concurrency {

  SerialTaskQueueChain::SerialTaskQueueChain(
    vector<shared_ptr<SerialTaskQueue>> queues)
    : queues_{move(queues)}
  {}

} // namespace hep::concurrency
