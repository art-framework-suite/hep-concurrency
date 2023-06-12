#ifndef hep_concurrency_cache_fwd_h
#define hep_concurrency_cache_fwd_h

#include "cetlib_except/cxx20_macros.h"
#if CET_CONCEPTS_AVAILABLE
#include <concepts>
#endif
#include <cstddef>
#include <functional>
#include "detail/cache_hashers.h"

namespace hep::concurrency {

  namespace detail {
    #if CET_CONCEPTS_AVAILABLE
    template <typename Key>
    concept hashable_cache_key =
      requires (Key key) {
      { detail::counter_hasher<Key>{}(key) } -> std::convertible_to<std::size_t>;
      { detail::collection_hasher<Key>::hash(key) } -> std::convertible_to<std::size_t>;
    };
    #endif

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
