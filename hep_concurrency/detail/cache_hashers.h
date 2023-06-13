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
#include "cetlib_except/cxx20_macros.h"
#if CET_CONCEPTS_AVAILABLE
#include <concepts>
#endif
#include <cstddef>
#include <functional>

namespace hep::concurrency::detail {

  #if CET_CONCEPTS_AVAILABLE
  template <std::equality_comparable Key>
  #else
  template <typename Key>
  #endif
  struct collection_hasher_base {
    static bool equal (Key const& a, Key const& b)
      {
        return a == b;
      }
  };

  #if CET_CONCEPTS_AVAILABLE
  template <typename Key>
  concept has_std_hash_spec = requires (Key key) {
    { std::hash<Key>{}(key) } -> std::convertible_to<std::size_t>;
  };

  template <typename Key>
  concept has_hash_function = requires (Key key) {
    { key.hash() } -> std::convertible_to<std::size_t>;
  };
  #endif

  template <typename Key>
  struct collection_hasher : collection_hasher_base<Key> {
  #if CET_CONCEPTS_AVAILABLE
  };

  template <typename Key>
  concept hashable_cache_key =
    has_std_hash_spec<Key> || has_hash_function<Key>;

  // Satisfies tbb::concurrent_hash_map
  template <has_std_hash_spec Key>
  struct collection_hasher<Key> : collection_hasher_base<Key>{
    using collection_hasher_base<Key>::equal;
  #endif
    static size_t
    hash(Key const& key)
    {
  #if ! CET_CONCEPTS_AVAILABLE
      // std::hash specializations are...special--although a given
      // specialization may exist, it may be "disabled".  A disabled
      // specialization will not be default-constructible (among other
      // things).  Here, we assume a default-constructible
      // specialization is sufficient to model the hash concept, even
      // though the full concept is more constrained.
      if constexpr (std::is_default_constructible_v<std::hash<Key>>) {
  #endif
        std::hash<Key> hasher;
        return hasher(key);
  #if ! CET_CONCEPTS_AVAILABLE
      }
  #else
    }
  };

  template <has_hash_function Key>
  struct collection_hasher<Key> : collection_hasher_base<Key> {
    using collection_hasher_base<Key>::equal;
    static size_t
    hash(Key const& key)
    {
  #endif
  #if ! CET_CONCEPTS_AVAILABLE
      else {
  #endif
        return key.hash();
  #if ! CET_CONCEPTS_AVAILABLE
      }
  #endif
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
