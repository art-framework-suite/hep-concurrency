#include "hep_concurrency/cache.h"

#include <string>

namespace {
  struct not_hashable {
    bool operator==(not_hashable const& other) const { return data == other.data; }
    int data { 3 };
  };
}

int main() {
  hep::concurrency::cache<not_hashable, int> cache;
  #if ! CET_CONCEPTS_ENABLED
  // Without concepts, need to call a disabled function.
  cache.emplace(not_hashable{}, 0);
  #endif
  return 0;
}
