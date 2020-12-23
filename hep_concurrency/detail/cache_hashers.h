#ifndef hep_concurrency_detail_cache_hashers_h
#define hep_concurrency_detail_cache_hashers_h

// ===================================================================

// TBB's concurrent_hash_map and concurrent_unordered_map class
// templates specify their hashing requirements differently.  For a
// user-specified key to support both map templates, a hash value and
// an equality function must be accesible.
//
// This file contains the metaprogramming utilities required to
// support both map templates without the user needing to specify
// different hashing implementations.  For keys that do not already
// have implementations provided by the C++ STL, the following member
// functions must be provided:
//
//   1. size_t Key::hash() const
//   2. bool Key::operator==(Key const&) const

// =================================================================

#include <type_traits>
#include <utility>

namespace hep::concurrency::detail {

  // Satisfies tbb::concurrent_hash_map
  template <typename Key>
  struct collection_hasher {
    static size_t
    hash(Key const& key)
    {
      // std::hash specializations are...special--although a given
      // specialization may exist, it may be "disabled".  A disabled
      // specialization will not be default-constructible (among other
      // things).  Here, we assume a default-constructible
      // specialization is sufficient to model the hash concept, even
      // though the full concept is more constrained.
      if constexpr (std::is_default_constructible_v<std::hash<Key>>) {
        std::hash<Key> hasher;
        return hasher(key);
      } else {
        return key.hash();
      }
    }
    static bool
    equal(Key const& a, Key const& b)
    {
      return a == b;
    }
  };

  // Satisfies tbb::concurrent_unordered_map
  template <typename Key>
  struct counter_hasher {
    size_t
    operator()(Key const& key) const
    {
      return collection_hasher<Key>::hash(key);
    }
  };

}

#endif /* hep_concurrency_detail_cache_hashers_h */

// Local Variables:
// mode: c++
// End:
