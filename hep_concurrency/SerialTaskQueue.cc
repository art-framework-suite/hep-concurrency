// vim: set sw=2 expandtab :
#include "hep_concurrency/SerialTaskQueue.h"

#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/tsan.h"
#include "tbb/task.h"

#include <atomic>
#include <queue>

using namespace std;

namespace hep::concurrency {

  bool
  SerialTaskQueue::pause()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    return ++pauseCount_ == 1;
  }

  bool
  SerialTaskQueue::resume()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
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
    RecursiveMutexSentry sentry{mutex_, __func__};
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
    RecursiveMutexSentry sentry{mutex_, __func__};
    taskRunning_ = false;
    return pickNextTask();
  }

  tbb::task*
  SerialTaskQueue::pickNextTask()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
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
