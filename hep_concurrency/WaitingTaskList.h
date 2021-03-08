#ifndef hep_concurrency_WaitingTaskList_h
#define hep_concurrency_WaitingTaskList_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/WaitingTask.h"

#include <exception>
#include <mutex>
#include <queue>

#include <tbb/task_group.h> // Can't forward-declare this class.

namespace hep::concurrency {

  class WaitingTaskList {
  public:
    explicit WaitingTaskList(tbb::task_group& group);

    // Disable copy operations
    WaitingTaskList(WaitingTaskList const&) = delete;
    WaitingTaskList& operator=(WaitingTaskList const&) = delete;

    void add(WaitingTaskPtr);
    void doneWaiting(std::exception_ptr ex_ptr = {});
    void reset();

  private:
    void runAllTasks_();

    tbb::task_group* taskGroup_;
    std::recursive_mutex mutex_{};
    std::queue<WaitingTaskPtr> taskQueue_{};
    bool waiting_{true};
    std::exception_ptr exceptionPtr_{};
  };

} // namespace hep::concurrency

#endif /* hep_concurrency_WaitingTaskList_h */

// Local Variables:
// mode: c++
// End:
