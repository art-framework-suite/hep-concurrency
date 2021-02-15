// vim: set sw=2 expandtab :
#include "hep_concurrency/SerialTaskQueue.h"

namespace hep::concurrency {

  bool
  SerialTaskQueue::pause()
  {
    std::lock_guard sentry{mutex_};
    return ++pauseCount_ == 1;
  }

  bool
  SerialTaskQueue::resume()
  {
    std::lock_guard sentry{mutex_};
    if (--pauseCount_ != 0) {
      return false;
    }

    if (auto next_task = pickNextTask()) {
      group_->run(*next_task);
    }
    return true;
  }

  auto
  SerialTaskQueue::pickNextTask() -> task_ptr_t
  {
    std::lock_guard sentry{mutex_};
    if ((pauseCount_ == 0) && !taskRunning_) {
      if (!taskQueue_.empty()) {
        auto ret = taskQueue_.front();
        taskQueue_.pop();
        taskRunning_ = true;
        return ret;
      }
    }
    return nullptr;
  }

  auto
  SerialTaskQueue::notify() -> task_ptr_t
  {
    std::lock_guard sentry{mutex_};
    taskRunning_ = false;
    return pickNextTask();
  }

  void
  SerialTaskQueue::notify_and_run()
  {
    if (auto next_task = notify()) {
      group_->run(*next_task);
    }
  }

} // namespace hep::concurrency
