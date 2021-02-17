// vim: set sw=2 expandtab :
#include "hep_concurrency/WaitingTaskList.h"

#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/hardware_pause.h"
#include "hep_concurrency/tsan.h"
#include "tbb/task_group.h"

#include <atomic>
#include <cassert>
#include <exception>
#include <queue>

using namespace std;

namespace hep::concurrency {

  WaitingTaskList::WaitingTaskList(tbb::task_group& group) : taskGroup_{&group}
  {}

  void
  WaitingTaskList::reset()
  {
    std::lock_guard sentry{mutex_};
    // There should be no tasks that still need to run.
    assert(taskQueue_.empty());
    waiting_ = true;
    exceptionPtr_ = {};
  }

  void
  WaitingTaskList::add(WaitingTaskPtr tsk)
  {
    assert(tsk != nullptr);
    std::lock_guard sentry{mutex_};
    tsk->increment_ref_count();
    if (!waiting_) {
      // We are not in waiting mode, we should run the task
      // immediately.
      if (exceptionPtr_) {
        // The doneWaiting call that set us running propagated an
        // exception to us.
        tsk->dependentTaskFailed(exceptionPtr_);
      }
      if (tsk->decrement_ref_count() == 0) {
        // Mind the task ownership!
        taskGroup_->run([task = std::move(tsk)] { (*task)(); });
      }
      // Note: If we did not spawn the task above, then we assume that
      // the same task is on multiple waiting task lists, and
      // whichever list decrements the refcount to zero will be the
      // one that actually spawns it.
      return;
    }
    taskQueue_.push(tsk);
  }

  void
  WaitingTaskList::doneWaiting(exception_ptr exc)
  {
    std::lock_guard sentry{mutex_};
    // Run all the tasks and propagate an exception to them.
    waiting_ = false;
    exceptionPtr_ = exc;
    runAllTasks_();
  }

  void
  WaitingTaskList::runAllTasks_()
  {
    std::lock_guard sentry{mutex_};
    auto isEmpty = taskQueue_.empty();
    while (!isEmpty) {
      auto tsk = taskQueue_.front();
      taskQueue_.pop();
      if (exceptionPtr_) {
        tsk->dependentTaskFailed(exceptionPtr_);
      }
      if (tsk->decrement_ref_count() == 0) {
        // Mind the task ownership!
        taskGroup_->run([task = std::move(tsk)] { (*task)(); });
      }
      // Note: If we did not spawn the task above, then we assume that
      // the same task is on multiple waiting task lists, and
      // whichever list decrements the refcount to zero will be the
      // one that actually spawns it.
      isEmpty = taskQueue_.empty();
    }
  }

} // namespace hep::concurrency
