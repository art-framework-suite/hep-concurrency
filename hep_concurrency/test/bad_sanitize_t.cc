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
  thread_sanitize<func> test_sanitize("invalid argument");
}
