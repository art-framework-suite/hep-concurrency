#include "hep_concurrency/cache.h"

#include <string>

namespace {
  class no_eq {
  private:
    int data;
  };
}

int
main()
{
  hep::concurrency::cache<no_eq, int> cache;
#if !CET_CONCEPTS_ENABLED
  // Without concepts, need to call a disabled function.
  cache.emplace(no_eq{}, 0);
#endif
  return 0;
}
