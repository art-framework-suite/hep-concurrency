#ifndef hep_concurrency_SerialTaskQueue_h
#define hep_concurrency_SerialTaskQueue_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/tsan.h"
#include "tbb/task.h"

#include <atomic>
#include <queue>

namespace hep::concurrency {

  class SerialTaskQueue final {
  public:
    SerialTaskQueue() = default;

    // Disable copy operations
    SerialTaskQueue(SerialTaskQueue const&) = delete;
    SerialTaskQueue& operator=(SerialTaskQueue const&) = delete;

    template <typename F>
    void push(F&& func);

    bool pause();
    bool resume();

    // Used only by QueuedTask<T>::execute().
    tbb::task* notify();

  private:
    void pushTask(tbb::task*);
    tbb::task* pickNextTask();

    hep::concurrency::RecursiveMutex mutex_{"SerialTaskQueue::mutex_"};
    std::queue<tbb::task*> taskQueue_{};
    unsigned long pauseCount_{};
    bool taskRunning_{false};
  };

  template <typename F>
  class QueuedTask final : public tbb::task {
  public:
    QueuedTask(SerialTaskQueue*, F&& func);
    ~QueuedTask() override;

  private:
    tbb::task* execute() override;

    // Used to call notify() when the functor returns.
    std::atomic<SerialTaskQueue*> queue_;
    F func_;
  };

  template <typename F>
  QueuedTask<F>::~QueuedTask()
  {
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    ANNOTATE_THREAD_IGNORE_BEGIN;
    queue_ = nullptr;
    ANNOTATE_THREAD_IGNORE_END;
  }

  template <typename F>
  QueuedTask<F>::QueuedTask(SerialTaskQueue* queue, F&& func)
    : queue_{queue}, func_{std::forward<F>(func)}
  {}

  template <typename F>
  tbb::task*
  QueuedTask<F>::execute()
  {
    try {
      func_();
    }
    catch (...) {
    }
    auto ret = queue_.load()->notify();
    if (ret != nullptr) {
      tbb::task::spawn(*ret);
    }
    return nullptr;
  }

  template <typename F>
  void
  SerialTaskQueue::push(F&& func)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    ANNOTATE_THREAD_IGNORE_BEGIN;
    tbb::task* p = new (tbb::task::allocate_root())
      QueuedTask<F>{this, std::forward<F>(func)};
    ANNOTATE_THREAD_IGNORE_END;
    pushTask(p);
  }

} // namespace hep::concurrency

#endif /* hep_concurrency_SerialTaskQueue_h */

// Local Variables:
// mode: c++
// End:
