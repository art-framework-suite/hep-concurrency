#include <catch2/catch_test_macros.hpp>

#include "hep_concurrency/thread_sanitize.h"

using namespace hep::concurrency;

namespace {
  struct Obj {
    explicit Obj(int) {}
  };

  template <typename T, typename... ARGS>
  concept can_thread_sanitize =
  requires (ARGS&&... args)
  { { thread_sanitize<T>{std::forward<ARGS>(args)...} }; };

  template <typename T, typename... ARGS>
  constexpr bool verify_thread_sanitize(ARGS&&...) {
    return can_thread_sanitize<T, ARGS...>;
  };
}

TEST_CASE("Enforce thread_sanitize() constraints")
{
  CHECK(verify_thread_sanitize<Obj>(3));
  CHECK_FALSE(verify_thread_sanitize<Obj>());
  CHECK_FALSE(verify_thread_sanitize<Obj>("invalid argument"));
}
