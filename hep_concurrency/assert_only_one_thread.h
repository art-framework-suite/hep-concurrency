#ifndef hep_concurrency_assert_only_one_thread_h
#define hep_concurrency_assert_only_one_thread_h
// vim: set sw=2 expandtab :

// ===================================================================
// HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD()
//
// This macro is a utility that can be called wherever it is expected
// for only one thread to be accessing a particular block of code.
//
// It has similar semantics to the standard 'assert' macro:
//
//   - It is disabled if NDEBUG is defined
//   - If more than one thread is accessing that block of code at one
//     time, std::abort is called.
//   - It is encouraged to be used whenever single-threaded execution
//     of a code block is a pre-condition.
//
// If the std::abort() is called, file, line-number, and function-name
// information will be provided.
// ===================================================================

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <source_location>
#include <string>

namespace hep::concurrency::detail {

  class thread_counter {
  public:
    explicit thread_counter(char const* filename,
                            unsigned const linenum,
                            char const* funcname)
      : filename_{filename}, linenum_{linenum}, funcname_{funcname}
    {}

    class sentry {
    public:
      ~sentry() noexcept { --tc_.counter_; }
      sentry(thread_counter& tc, bool const terminate = true) : tc_{tc}
      {
        if (++tc_.counter_ == 1u) {
          return;
        }
        std::cerr << "Failed assert--more than one thread accessing location:\n"
                  << "  " << tc_.filename_ << ':' << tc_.linenum_ << '\n'
                  << "  function: " << tc_.funcname_ << '\n';
        if (terminate) {
          std::abort();
        }
      }

    private:
      thread_counter& tc_;
    };

  private:
    std::string const filename_;
    unsigned const linenum_;
    std::string const funcname_;
    std::atomic<unsigned> counter_{0u};
  };

} // namespace hep::concurrency::detail

#ifdef NDEBUG
#define HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD()
#else // NDEBUG
#define HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD()                               \
  static hep::concurrency::detail::thread_counter s_tc_{                       \
    std::source_location::current().file_name(),                               \
    std::source_location::current().line(),                                    \
    std::source_location::current().function_name()};                          \
  hep::concurrency::detail::thread_counter::sentry sentry_tc_                  \
  {                                                                            \
    s_tc_                                                                      \
  }
#endif // NDEBUG

#endif /* hep_concurrency_assert_only_one_thread_h */

// Local variables:
// mode: c++
// End:
