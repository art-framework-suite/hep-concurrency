#ifndef hep_concurrency_cache_fwd_h
#define hep_concurrency_cache_fwd_h

namespace hep::concurrency {

  namespace detail {
    template <typename Key, typename Value>
    class cache_impl;
  }

  template <typename Key, typename Value>
  using cache = detail::cache_impl<Key, Value>;
}

#endif /* hep_concurrency_cache_fwd_h */

// Local Variables:
// mode: c++
// End:
