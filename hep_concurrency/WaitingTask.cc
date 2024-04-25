#include "hep_concurrency/WaitingTask.h"
// vim: set sw=2 expandtab :

#include <atomic>
#include <exception>

using namespace std;

namespace hep::concurrency {

  WaitingTaskPtr
  make_waiting_task(task_func_t&& t)
  {
    return std::make_shared<WaitingTask>(std::forward<task_func_t>(t));
  }

  WaitingTaskPtr
  make_waiting_task(task_func_t&& t, unsigned n_signals)
  {
    return std::make_shared<WaitingTask>(std::forward<task_func_t>(t),
                                         n_signals);
  }

  std::exception_ptr
  WaitingTask::exceptionPtr() const
  {
    auto ret = ptr_.load();
    return ret ? *ret : std::exception_ptr{};
  }

  void
  WaitingTask::dependentTaskFailed(std::exception_ptr iPtr)
  {
    if (iPtr && (ptr_.load() == nullptr)) {
      auto temp = new std::exception_ptr(iPtr);
      std::exception_ptr* expected = nullptr;
      if (ptr_.compare_exchange_strong(expected, temp)) {
        return;
      }
      delete temp;
      temp = nullptr;
    }
  }

} // hep::concurrency
