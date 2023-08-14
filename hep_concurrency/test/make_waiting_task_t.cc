#include <catch2/catch_test_macros.hpp>

#include "hep_concurrency/WaitingTask.h"

#include <concepts>

namespace {
  class GoodTask {
  public:
    GoodTask(int num) : num_(num) {}
    void
    operator()(std::exception_ptr)
    {}

  private:
    int num_{};
  };

  void
  goodFunc(std::exception_ptr)
  {}

  void
  badFunc()
  {}

  template <typename T, typename... ARGS>
  concept can_make_waiting_task =
    requires(ARGS&&... args) {
      hep::concurrency::make_waiting_task<T>(std::forward<ARGS>(args)...);
    } || requires(T&& t, ARGS&&... args) {
           hep::concurrency::make_waiting_task(std::forward<T>(t),
                                               std::forward<ARGS>(args)...);
         };

  template <typename T, typename... ARGS>
  struct verify_waiting_task_compatible_t : std::false_type {};

  template <typename T, typename... ARGS>
    requires can_make_waiting_task<T, ARGS...>
  struct verify_waiting_task_compatible_t<T, ARGS...> : std::true_type {};

  template <typename T, typename... ARGS>
  constexpr bool verify_waiting_task_compatible_v =
    verify_waiting_task_compatible_t<T, ARGS...>::value;

  template <typename T>
  constexpr bool
  verify_waiting_task_compatible(T&&, unsigned = 0u)
  {
    return verify_waiting_task_compatible_v<T>;
  }
  template <typename T, typename... ARGS>
  constexpr bool
  verify_waiting_task_compatible(ARGS&&...)
  {
    return verify_waiting_task_compatible_v<T, ARGS...>;
  }
}

TEST_CASE("Enforcing waiting task constraints")
{
  CHECK(verify_waiting_task_compatible(goodFunc));
  CHECK_FALSE(verify_waiting_task_compatible(badFunc));
  CHECK(verify_waiting_task_compatible<GoodTask>(1));
  CHECK_FALSE(verify_waiting_task_compatible<GoodTask>(1, 2, 3));
  CHECK(verify_waiting_task_compatible(GoodTask{1}));
  CHECK(verify_waiting_task_compatible(GoodTask{1}, 1u));
}
