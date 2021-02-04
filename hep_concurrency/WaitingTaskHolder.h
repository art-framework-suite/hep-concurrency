#ifndef hep_concurrency_WaitingTaskHolder_h
#define hep_concurrency_WaitingTaskHolder_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/WaitingTask.h"

#include <atomic>

namespace hep::concurrency {

  class WaitingTaskHolder {
  public:
    explicit WaitingTaskHolder(tbb::task*);
    ~WaitingTaskHolder();

    void doneWaiting(std::exception_ptr ex_ptr = {});

  private:
    std::atomic<tbb::task*> m_task{nullptr};
  };

} // namespace hep::concurrency

#endif /* hep_concurrency_WaitingTaskHolder_h */

// Local Variables:
// mode: c++
// End:
