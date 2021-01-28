// vim: set sw=2 expandtab :
#include "hep_concurrency/WaitingTaskHolder.h"

#include "hep_concurrency/WaitingTask.h"

namespace hep::concurrency {

  WaitingTaskHolder::WaitingTaskHolder(tbb::task* iTask)
  {
    auto wt = dynamic_cast<WaitingTaskExHolder*>(iTask);
    if (wt == nullptr) {
      abort();
    }
    iTask->increment_ref_count();
    m_task = iTask;
  }

  WaitingTaskHolder::~WaitingTaskHolder()
  {
    if (m_task.load() != nullptr) {
      doneWaiting(std::exception_ptr{});
    }
  }

  void
  WaitingTaskHolder::doneWaiting(std::exception_ptr iExcept)
  {
    if (iExcept) {
      auto wt = dynamic_cast<WaitingTaskExHolder*>(m_task.load());
      if (wt == nullptr) {
        abort();
      }
      wt->dependentTaskFailed(iExcept);
    }
    if (m_task.load()->decrement_ref_count() == 0) {
      tbb::task::spawn(*m_task.load());
    }
    m_task = nullptr;
  }

} // namespace hep::concurrency
