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

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace hep::concurrency::detail {

  template <std::equality_comparable Key>
  struct collection_hasher_base {
    static bool
    equal(Key const& a, Key const& b)
    {
      return a == b;
    }
  };

  template <typename Key>
  concept has_std_hash_spec = requires(Key key) {
                                {
                                  std::hash<Key>{}(key)
                                  } -> std::convertible_to<std::size_t>;
                              };

  template <typename Key>
  concept has_hash_function = requires(Key key) {
                                {
                                  key.hash()
                                  } -> std::convertible_to<std::size_t>;
                              };

  template <typename Key>
  concept hashable_cache_key = has_std_hash_spec<Key> || has_hash_function<Key>;

  template <hashable_cache_key Key>
  struct collection_hasher : collection_hasher_base<Key> {
    using collection_hasher_base<Key>::equal;

    static size_t
    hash(Key const& key)
      requires has_std_hash_spec<Key>
    {
      std::hash<Key> hasher;
      return hasher(key);
    }

    static size_t
    hash(Key const& key)
      requires has_hash_function<Key>
    {
      return key.hash();
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
