#include <catch2/catch_test_macros.hpp>

#include "hep_concurrency/SerialTaskQueueChain.h"

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
  concept can_push_to_chained_queues =
    requires(hep::concurrency::SerialTaskQueueChain& chain, T t) {
      chain.push(t);
    };

  auto
  verify_push_to_chained_queues(auto t)
  {
    return can_push_to_chained_queues<decltype(t)>;
  }
}

TEST_CASE("Enforce task constraints")
{
  CHECK(verify_push_to_chained_queues(GoodTask{3}));
  CHECK(verify_push_to_chained_queues(goodFunc));
  CHECK_FALSE(verify_push_to_chained_queues(num));
}
