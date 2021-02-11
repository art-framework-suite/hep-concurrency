// vim: set sw=2 expandtab :
#include "hep_concurrency/SerialTaskQueue.h"

#include "tbb/task.h"

#include <atomic>
#include <queue>

using namespace std;

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

    tbb::task* nt = pickNextTask();
    if (nt != nullptr) {
      tbb::task::spawn(*nt);
    }
    return true;
  }

  void
  SerialTaskQueue::pushTask(tbb::task* tsk)
  {
    std::lock_guard sentry{mutex_};
    if (tsk == nullptr) {
      return;
    }

    taskQueue_.push(tsk);
    tbb::task* nt = pickNextTask();
    if (nt != nullptr) {
      tbb::task::spawn(*nt);
    }
  }

  tbb::task*
  SerialTaskQueue::notify()
  {
    std::lock_guard sentry{mutex_};
    taskRunning_ = false;
    return pickNextTask();
  }

  tbb::task*
  SerialTaskQueue::pickNextTask()
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

} // namespace hep::concurrency
