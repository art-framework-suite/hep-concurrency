#ifndef hep_concurrency_cache_fwd_h
#define hep_concurrency_cache_fwd_h

#include "detail/cache_hashers.h"

namespace hep::concurrency {
  template <detail::hashable_cache_key Key, typename Value>
  class cache;
}

#endif /* hep_concurrency_cache_fwd_h */

// Local Variables:
// mode: c++
// End:
