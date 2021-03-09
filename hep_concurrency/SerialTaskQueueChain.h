#ifndef hep_concurrency_SerialTaskQueueChain_h
#define hep_concurrency_SerialTaskQueueChain_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/SerialTaskQueue.h"

#include <atomic>
#include <cassert>
#include <exception>
#include <memory>
#include <mutex>
#include <vector>

namespace hep::concurrency {

  class SerialTaskQueueChain {
  public:
    explicit SerialTaskQueueChain(
      std::vector<std::shared_ptr<SerialTaskQueue>>);

    // No copy/move operations
    SerialTaskQueueChain(SerialTaskQueueChain const&) = delete;
    SerialTaskQueueChain(SerialTaskQueueChain&&) = delete;
    SerialTaskQueueChain& operator=(SerialTaskQueueChain const&) = delete;
    SerialTaskQueueChain& operator=(SerialTaskQueueChain&&) = delete;

    template <typename F>
    void push(F&&);

  private:
    template <typename F>
    void passDown(unsigned int, F&&);
    template <typename F>
    void runFunc(F const&);

    std::recursive_mutex mutex_{};
    std::vector<std::shared_ptr<SerialTaskQueue>> queues_;
  };

  template <typename F>
  void
  SerialTaskQueueChain::push(F&& func)
  {
    std::lock_guard sentry{mutex_};
    assert(queues_.size() > 0);
    if (queues_.size() == 1) {
      queues_[0]->push([this, f = std::forward<F>(func)] { runFunc(f); });
    } else {
      queues_[0]->push([this, f = std::forward<F>(func)]() mutable {
        passDown(1, std::forward<F>(f));
      });
    }
  }

  template <typename F>
  void
  SerialTaskQueueChain::passDown(unsigned int idx, F&& func)
  {
    std::lock_guard sentry{mutex_};
    queues_[idx - 1]->pause();
    if ((idx + 1) == queues_.size()) {
      queues_[idx]->push([this, f = std::forward<F>(func)] { runFunc(f); });
    } else {
      auto nxt = idx + 1;
      queues_[idx]->push([this, nxt, f = std::forward<F>(func)]() mutable {
        passDown(nxt, std::forward<F>(f));
      });
    }
  }

  template <typename F>
  void
  SerialTaskQueueChain::runFunc(F const& func)
  {
    try {
      func();
    }
    catch (...) {
      std::lock_guard sentry{mutex_};
      for (auto it = queues_.rbegin() + 1; it != queues_.rend(); ++it) {
        (*it)->resume();
      }
      throw;
    }
    std::lock_guard sentry{mutex_};
    for (auto it = queues_.rbegin() + 1; it != queues_.rend(); ++it) {
      (*it)->resume();
    }
  }

} // namespace hep::concurrency

#endif /* hep_concurrency_SerialTaskQueueChain_h */

// Local Variables:
// mode: c++
// End:
