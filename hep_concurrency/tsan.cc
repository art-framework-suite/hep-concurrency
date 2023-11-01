#include "hep_concurrency/tsan.h"

// #include <x86intrin.h>

namespace hep::concurrency {
  int intentionalDataRace_{0};
  thread_local int ignoreBalance_{0};
}
