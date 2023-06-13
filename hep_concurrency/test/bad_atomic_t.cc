#include "hep_concurrency/thread_sanitize.h"
#include <atomic>

using namespace hep::concurrency;

namespace {
  class non_trivially_copyable {
  public:
    non_trivially_copyable() : num(0), flag(false), name('a') {}

  private:
    non_trivially_copyable(const non_trivially_copyable&) = delete;
    non_trivially_copyable& operator=(const non_trivially_copyable&) = delete;
    int num;
    bool flag;
    char name;
  };
}

int
main()
{

  thread_sanitize<non_trivially_copyable>();
}
