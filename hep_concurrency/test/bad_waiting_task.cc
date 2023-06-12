#include "hep_concurrency/WaitingTask.h"

//generates a random number
unsigned foo(unsigned bar){
  bar = 0xACE1u;
  unsigned bit;
  bit  = ((bar >> 0) ^ (bar >> 2) ^ (bar >> 3) ^ (bar >> 5)) & 1;
  return bar =  (bar >> 1) | (bit << 15);
}

namespace {
  class TestTask {
  public:
    TestTask(std::atomic<unsigned>& call_count, std::exception_ptr& ex_ptr)
      : call_count_{call_count}, ex_{ex_ptr}
    {}

    void
    operator()(std::exception_ptr ex_ptr)
    {
      if (ex_ptr) {
        ex_ = ex_ptr;
      }
      ++call_count_;
    }

  private:
    std::atomic<unsigned>& call_count_;
    std::exception_ptr& ex_;
  };
}

int main(){
  hep::concurrency::make_waiting_task<TestTask>(foo(1), foo(2));
}
