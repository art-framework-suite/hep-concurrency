#ifndef hep_concurrency_WaitingTaskList_h
#define hep_concurrency_WaitingTaskList_h
// vim: set sw=2 expandtab :

#include <exception>
#include <mutex>
#include <queue>

namespace tbb {
  class task;
}

namespace hep::concurrency {

  class WaitingTaskList {
  public:
    WaitingTaskList();
    ~WaitingTaskList();

    // Disable copy operations
    WaitingTaskList(WaitingTaskList const&) = delete;
    WaitingTaskList& operator=(WaitingTaskList const&) = delete;

    void add(tbb::task*);
    void doneWaiting(std::exception_ptr ex_ptr = {});
    void reset();

  private:
    void runAllTasks_();

    std::recursive_mutex mutex_{};
    std::queue<tbb::task*>* taskQueue_;
    bool waiting_;
    std::exception_ptr* exceptionPtr_;
  };

} // namespace hep::concurrency

#endif /* hep_concurrency_WaitingTaskList_h */

// Local Variables:
// mode: c++
// End:
