#include "hep_concurrency/thread_sanitize.h"
#include <atomic>

using namespace hep::concurrency;

struct Obj {
  explicit Obj(int) {}
};

int
main()
{
  thread_sanitize<Obj> test_sanitize("invalid argument");
}
