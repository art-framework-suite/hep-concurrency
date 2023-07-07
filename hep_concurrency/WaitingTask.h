#ifndef hep_concurrency_WaitingTask_h
#define hep_concurrency_WaitingTask_h
// vim: set sw=2 expandtab :

#include <atomic>
#include <concepts>
#include <exception>
#include <functional>
#include <memory>

namespace hep::concurrency {

  using task_func_t = std::function<void(std::exception_ptr)>;

  namespace detail {
    template <typename T, typename... Args>
    concept waiting_task_compatible = requires(Args&&... args) {
                                        {
                                          T{std::forward<Args>(args)...}
                                          } -> std::convertible_to<task_func_t>;
                                      };
  }

  class WaitingTask {
  public:
    explicit WaitingTask(task_func_t&& f, unsigned n_signals = 1)
      : func_{std::move(f)}, n_{n_signals}
    {}

    // API required by tbb::task_group
    void
    operator()()
    {
      func_(exceptionPtr());
    }

    std::exception_ptr exceptionPtr() const;
    void dependentTaskFailed(std::exception_ptr);

    unsigned
    ref_count()
    {
      return count_;
    }

    void
    increment_ref_count()
    {
      ++count_;
    }
    unsigned
    decrement_ref_count()
    {
      --count_;
      return count_;
    }

    unsigned
    decrement_done_count()
    {
      --n_;
      return n_;
    }

  private:
    task_func_t func_;
    std::atomic<unsigned> n_;
    std::atomic<std::exception_ptr*> ptr_{nullptr};
    std::atomic<unsigned> count_{};
  };

  using WaitingTaskPtr = std::shared_ptr<WaitingTask>;

  template <typename T, typename... Args>
    requires detail::waiting_task_compatible<T, Args...>
  WaitingTaskPtr
  make_waiting_task(Args&&... args)
  {
    return std::make_shared<WaitingTask>(T{std::forward<Args>(args)...});
  }

} // hep::concurrency

#endif /* hep_concurrency_WaitingTask_h */

// Local Variables:
// mode: c++
// End:
