#ifndef hep_concurrency_SerialTaskQueue_h
#define hep_concurrency_SerialTaskQueue_h
// vim: set sw=2 expandtab :

#include "tbb/task_group.h"

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include "cetlib_except/cxx20_macros.h"
#if CET_CONCEPTS_AVAILABLE
#include <concepts>
#endif

namespace hep::concurrency {
  
  using task_t = std::function<void()>;

#if CET_CONCEPTS_AVAILABLE
  namespace detail {
    template<typename F>
    concept convertible_to_task_t = std::is_convertible_v<F, task_t>;
  }
#endif

  class SerialTaskQueue final {
  public:
    SerialTaskQueue(tbb::task_group& group) : group_{&group} {}

    // Disable copy operations
    SerialTaskQueue(SerialTaskQueue const&) = delete;
    SerialTaskQueue& operator=(SerialTaskQueue const&) = delete;

#if CET_CONCEPTS_AVAILABLE
    template <detail::convertible_to_task_t F>
#else
    template <typename F>
#endif
    void push(F&& func);

    bool pause();
    bool resume();

    // Used only by QueuedTask<T>::execute().
    void notify_and_run();

  private:
    class QueuedTask;
    using task_ptr_t = std::shared_ptr<QueuedTask>;

    auto pickNextTask() -> task_ptr_t;
    auto notify() -> task_ptr_t;

    tbb::task_group* group_;
    std::recursive_mutex mutex_{};
    std::queue<task_ptr_t> taskQueue_{};
    unsigned long pauseCount_{};
    bool taskRunning_{false};
  };

  class SerialTaskQueue::QueuedTask {
  public:
    QueuedTask(SerialTaskQueue* queue, task_t func)
      : queue_{queue}, func_{std::move(func)}
    {}

    explicit operator bool() const { return static_cast<bool>(func_); }

    void
    operator()() const
    {
      try {
        func_();
      }
      catch (...) {
      }
      queue_->notify_and_run();
    }

  private:
    SerialTaskQueue* queue_;
    task_t func_;
  };

#if CET_CONCEPTS_AVAILABLE
  template <detail::convertible_to_task_t F>
#else
  template <typename F>
#endif
  void SerialTaskQueue::push(F&& func)
  {
    std::lock_guard sentry{mutex_};
    taskQueue_.push(std::make_shared<QueuedTask>(this, std::forward<F>(func)));
    if (auto next_task = pickNextTask()) {
      group_->run(*next_task);
    }
  }

} // namespace hep::concurrency

#endif /* hep_concurrency_SerialTaskQueue_h */

// Local Variables:
// mode: c++
// End:
