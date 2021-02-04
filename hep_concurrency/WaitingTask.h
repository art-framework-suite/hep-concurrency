#ifndef hep_concurrency_WaitingTask_h
#define hep_concurrency_WaitingTask_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/tsan.h"
#include "tbb/task.h"

#include <atomic>
#include <exception>
#include <memory>

namespace hep::concurrency {

  class WaitingTaskExHolder {
  public:
    WaitingTaskExHolder();
    ~WaitingTaskExHolder();

    std::exception_ptr exceptionPtr() const;
    void dependentTaskFailed(std::exception_ptr);

  private:
    std::atomic<std::exception_ptr*> ptr_;
  };

  template <typename F>
  class FunctorWaitingTask : public tbb::task, public WaitingTaskExHolder {
  public:
    explicit FunctorWaitingTask(F&& f);

    // API required by tbb::task
    task* execute() override;

  private:
    F func_;
  };

  template <typename F>
  FunctorWaitingTask<F>::FunctorWaitingTask(F&& f) : func_{std::forward<F>(f)}
  {}

  template <typename F>
  tbb::task*
  FunctorWaitingTask<F>::execute()
  {
    func_(exceptionPtr());
    return nullptr;
  }

  template <typename ALLOC, typename F>
  tbb::task*
  make_waiting_task(ALLOC&& iAlloc, F&& f)
  {
    ANNOTATE_THREAD_IGNORE_BEGIN;
    auto ret = new (iAlloc) FunctorWaitingTask<F>(std::forward<F>(f));
    ANNOTATE_THREAD_IGNORE_END;
    return ret;
  }

} // hep::concurrency

#endif /* hep_concurrency_WaitingTask_h */

// Local Variables:
// mode: c++
// End:
