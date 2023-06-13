#ifndef hep_concurrency_thread_sanitize_h
#define hep_concurrency_thread_sanitize_h

// ======================================================================
// thread_sanitize and thread_sanitize_unique_ptr exist to encapsulate
// manipulations required to suppress false positives produced by the
// thread sanitizer.  Ideally, this class template would not exist,
// but since TBB reuses threads, the sanitizer thinks this is a
// problem.
// ======================================================================

#include "cetlib_except/cxx20_macros.h"
#include "hep_concurrency/tsan.h"
#if CET_CONCEPTS_AVAILABLE
#include <concepts>
#endif

#include <atomic>

#if CET_CONCEPTS_AVAILABLE
namespace detail {
  template <typename T>
  concept atomic_compatible =
    std::is_copy_constructible_v<T> && std::is_move_constructible_v<T> &&
    std::is_copy_assignable_v<T> && std::is_move_assignable_v<T>;
}
#endif

namespace hep {
  namespace concurrency {
#if CET_CONCEPTS_AVAILABLE
    template <detail::atomic_compatible T>
#else
    template <typename T>
#endif
    class thread_sanitize {
    public:
      template <typename... Args>
      thread_sanitize(Args&&... args)
      {
        obj_ = new T(std::forward<Args>(args)...);
      }

      operator T&() { return *obj_.load(); }

      thread_sanitize&
      operator=(T&& t)
      {
        *obj_.load() = std::forward<T>(t);
        return *this;
      }

      T*
      operator->() const
      {
        return obj_.load();
      }

      ~thread_sanitize() noexcept
      {
        ANNOTATE_THREAD_IGNORE_BEGIN;
        delete obj_.load();
        obj_ = nullptr;
        ANNOTATE_THREAD_IGNORE_END;
      }

    private:
      std::atomic<T*> obj_;
    };

#if CET_CONCEPTS_AVAILABLE
    template <detail::atomic_compatible T>
#else
    template <typename T>
#endif
    class thread_sanitize_unique_ptr {
    public:
      template <typename... Args>
      thread_sanitize_unique_ptr(T* const t)
      {
        obj_ = t;
      }

      operator T*() { return obj_.load(); }
      operator T const*() const { return obj_.load(); }
      operator bool() const { return obj_.load() != nullptr; }

      void
      reset(T* const t = nullptr)
      {
        delete obj_.load();
        obj_ = t;
      }

      T*
      get() const
      {
        return obj_.load();
      }
      T&
      operator*() const
      {
        return *get();
      }
      T*
      operator->() const
      {
        return get();
      }

      ~thread_sanitize_unique_ptr() noexcept
      {
        ANNOTATE_THREAD_IGNORE_BEGIN;
        reset();
        ANNOTATE_THREAD_IGNORE_END;
      }

    private:
      std::atomic<T*> obj_;
    };
  }
}

#endif /* hep_concurrency_thread_sanitize_h */

// Local Variables:
// mode: c++
// End:
