#include <catch2/catch_test_macros.hpp>

#include "hep_concurrency/SerialTaskQueue.h"

#include <concepts>
#include <fstream>

namespace {
  int
  num(int mun)
  {
    return mun;
  }
  void
  goodFunc()
  {}

  class GoodTask {
  public:
    GoodTask(int num) noexcept : num_(num) {}
    void
    operator()()
    {}

  private:
    int num_{};
  };

  template <typename T>
  concept can_push_to_queue =
    requires(hep::concurrency::SerialTaskQueue& q, T t) { q.push(t); };

  auto
  verify_push_to_queue(auto t)
  {
    return can_push_to_queue<decltype(t)>;
  }
}

TEST_CASE("Enforce task constraints")
{
  CHECK(verify_push_to_queue(GoodTask{3}));
  CHECK(verify_push_to_queue(goodFunc));
  CHECK_FALSE(verify_push_to_queue(num));
}
