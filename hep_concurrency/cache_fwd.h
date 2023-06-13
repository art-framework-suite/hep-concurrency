#ifndef hep_concurrency_cache_fwd_h
#define hep_concurrency_cache_fwd_h

#include "detail/cache_hashers.h"

namespace hep::concurrency {

  namespace detail {
    #if CET_CONCEPTS_AVAILABLE
    template <hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    class cache_impl;
  }

  template <typename Key, typename Value>
  using cache = detail::cache_impl<Key, Value>;
}

#endif /* hep_concurrency_cache_fwd_h */

// Local Variables:
// mode: c++
// End:
