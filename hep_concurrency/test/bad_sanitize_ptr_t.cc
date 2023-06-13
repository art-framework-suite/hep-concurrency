#include "hep_concurrency/thread_sanitize.h"
#include <atomic>

using namespace hep::concurrency;

struct func {
  int operator()(int num) {
    return num;
  }
};

int
main()
{
  func* test_ptr = new func{};
  thread_sanitize_unique_ptr<func> test_sanitize_unique_ptr(test_ptr);
}
