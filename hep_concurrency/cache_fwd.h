#ifndef hep_concurrency_cache_fwd_h
#define hep_concurrency_cache_fwd_h

#include "detail/cache_hashers.h"

namespace hep::concurrency {

  #if CET_CONCEPTS_AVAILABLE
  template <detail::hashable_cache_key Key, typename Value>
  #else
  template <typename Key, typename Value>
  #endif
  class cache;
}

#endif /* hep_concurrency_cache_fwd_h */

// Local Variables:
// mode: c++
// End:
