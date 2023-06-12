#ifndef hep_concurrency_cache_h
#define hep_concurrency_cache_h

// ===================================================================
//
// Overview
// --------
//
// The cache class template, implemented below, provides a means of
// caching data in a thread-safe manner, using TBB's
// concurrent_(unordered|hash)_map facilities.
//
// The user interface includes the cache and the cache_handle
// templates.  A cache_handle object is used to provid immutable
// access to the cache elements.  The cache entries are
// reference-counted so that an entry cannot be removed from the cache
// unless all handles referring to that object have been destroyed or
// invalidated.
//
// Cache cleanup and entry retention
// ---------------------------------
//
// To mitigate unnecessary memory usage, the cache's drop_unused()
// function may be called to remove all entries whose reference counts
// are zero.  It can be helpful to retain some unused entries in case
// they might be required again.  In that case, the
// drop_unused_but_last(n) function can be called, where n is an
// unsigned integer indicating the n "most recently created", yet
// unused, entries that should be retained.
//
// Concurrent operations
// ---------------------
//
// With the exception of shrink_to_fit, all member functions may be
// called concurrently.  Any locking is handled internally by TBB.  In
// order to provide the entry_for(...) functionality and not incur
// locking, an auxiliary data member was introduced that cannot shrink
// during concurrent processing.  This is likely to be a problem only
// if the number of total elements processed is very large.  Once
// serial access can be ensured, shrink_to_fit() may be called, which
// will remove all unused entries from the cache and reset the
// auxiliary data member to the appropriate size.
//
// entry_for(...) and user-defined key support
// -------------------------------------------
//
// It frequently happens that a set of data may apply for a range of
// values.  Instead of inserting an element into the cache for each
// value, the user may supply their own type as a key with the
// following interface (e.g.):
//
//   struct range_of_values {
//     unsigned start;
//     unsigned stop;
//
//     bool supports(unsigned const test_value) const
//     {
//        return start <= test_value && test_value < stop;
//     }
//   };
//
// If the user-defined type provides the 'bool supports(...) const'
// interface, then the cache's entry_for(...) interface is enabled,
// allowing users to retrieve the element (through a handle)
// corresponding to the key that supports a given value (e.g.):
//
//   cache<range_of_values, V> cache;
//   auto const my_key = range_of_values{0, 10};
//   cache.emplace(my_key, ...);
//   auto h = cache.entry_for(6); // Returns value for my_key
//
// N.B. The implementation assumes that for each 'entry_for(value)'
//      call, only one cache element's key.supports(...) function may
//      return true.  It is a runtime error for more than one key to
//      support the same value.
//
// Hashing and equality
// --------------------
//
// The cache implementation requires that the key supports:
//
//   1. A reproducible hash value for a given key value
//   2. Equality comparison (e.g. key1 == key2)
//
// These requirements are satisfied by many types provided by the C++
// language or STL.  However, there are some types for which an
// equality comparison may provided by the C++ STL, but for which a
// hash is not available (e.g. std::pair<unsigned, unsigned>).  In
// such cases, an explicit specialization of std::hash will be needed.
//
// For user-defined keys, the cache template will provide the hashing
// functionality required if the following member functions are
// provided in the key type:
//
//   struct range_of_values {
//     ...
//     size_t hash() const {...};
//     bool operator==(range_of_values const& rov) const {...};
//   };
//
// Technical notes
// ---------------
//
// Each cache entry is constructed with an identifier represented by
// an unsigned integer of type std::size_t. The identifier starts at 0
// and atomically increments by 1 for each new entry throughout the
// lifetime of the cache.  This choice makes it possible to retain n
// unused entries, as described above.
//
// This choice also implies that for each cache object, no more than
// std::numerical_limits<std::size_t>::max() - 1 entries may be
// created, a limit which is unlikely to ever be reached.
//
// ===================================================================

#include "cetlib_except/exception.h"
#include "hep_concurrency/assert_only_one_thread.h"
#include "hep_concurrency/cache_fwd.h"
#include "hep_concurrency/cache_handle.h"
#include "hep_concurrency/detail/cache_entry.h"
#include "hep_concurrency/detail/cache_hashers.h"
#include "hep_concurrency/detail/cache_key_supports.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_unordered_map.h"

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>

namespace hep::concurrency {

  namespace detail {

    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    class cache_impl {
      using count_map_t =
        tbb::concurrent_unordered_map<Key,
                                      detail::entry_count_ptr,
                                      detail::counter_hasher<Key>>;
      using count_value_type = typename count_map_t::value_type;
      using collection_t =
        tbb::concurrent_hash_map<Key,
                                 detail::cache_entry<Value>,
                                 detail::collection_hasher<Key>>;
      using accessor = typename collection_t::accessor;
  
    public:
      using mapped_type = typename collection_t::mapped_type;
      using value_type = typename collection_t::value_type;
      using handle = cache_handle<Key, Value>;

      // Concurrent operations
      // ---------------------

      handle at(Key const& key) const;

      // For key types that provide a 'supports' function, the user can
      // supply a value of type T, which will then be used to identify
      // and return a handle to the correct cache entry.
      template <typename T>
      handle entry_for(T const& t) const;

      // To optimize lookup, one can provide a handle as a hint, which
      // will be consulted first to determine if the hint can satisfy
      // the lookup for the provided type T.  If not, then the function
      // falls back to the entry_for function above.
      //
      // Calling this function can be more efficient than calling
      // at(key) for a handle that already points to the correct entry.
      template <typename T>
      handle entry_for(handle hint, T const& t) const;

      template <typename T>
      std::enable_if_t<std::is_convertible_v<T, Value>, handle> emplace(
        Key const& k,
        T&& value);

      // Memory mitigations that remove unused cache entries
      void drop_unused();
      void drop_unused_but_last(std::size_t const keep_last);

      // Thread-safe, but no synchronization
      // -----------------------------------
      size_t
      size() const
        {
          return std::size(entries_);
        }
      bool
      empty() const
        {
          return std::empty(entries_);
        }
      size_t
      capacity() const
        {
          return std::size(counts_);
        }

      // Thread-unsafe
      // -------------
      // Can be called only when serialized access to the cache is
      // guaranteed.
      void shrink_to_fit();

  private:
      std::vector<std::pair<std::size_t, Key>>
      unused_entries_()
        {
          std::vector<std::pair<std::size_t, Key>> result;
          for (auto const& [key, count] : counts_) {
            if (count->use_count == 0u) {
              result.emplace_back(count->sequence_number, key);
            }
          }
          return result;
        }

      std::atomic<std::size_t> next_sequence_number_{0ull};
      collection_t entries_;
      count_map_t counts_;
    };


    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    template <typename T>
    std::enable_if_t<std::is_convertible_v<T, Value>, cache_handle<Key, Value>>
    cache_impl<Key, Value>::emplace(Key const& key, T&& value)
    {
      // Lock held on key's map entry until the function returns.
      accessor access_token;
      if (not entries_.insert(access_token, key)) {
        // Entry already exists; return cached entry.
        return handle{&access_token->first, &access_token->second};
      }

      auto const sequence_number = next_sequence_number_.fetch_add(1);
      auto counter = detail::make_counter(sequence_number);
      access_token->second = mapped_type{std::forward<T>(value), counter};

      auto [it, inserted] = counts_.insert(count_value_type{key, counter});
      if (not inserted) {
        it->second = counter;
      }
      return handle{&access_token->first, &access_token->second};
    }

    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    cache_handle<Key, Value>
    cache_impl<Key, Value>::at(Key const& key) const
    {
      if (accessor access_token; entries_.find(access_token, key))
        return handle{&access_token->first, &access_token->second};
      return handle::invalid();
    }

    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    template <typename T>
    cache_handle<Key, Value>
    cache_impl<Key, Value>::entry_for(T const& t) const
    {
      static_assert(detail::valid_supports_expression_v<Key, T>,
                    "The Key type does not provide a const-qualified 'supports' "
                    "function that takes an argument of the provided type.");
      std::vector<Key> matching_keys;
      for (auto const& [key, count] : counts_) {
        if (key.supports(t)) {
          matching_keys.push_back(key);
        }
      }

      if (std::empty(matching_keys)) {
        return handle::invalid();
      }

      if (std::size(matching_keys) > 1) {
        throw cet::exception("Data retrieval error.")
          << "More than one key match.";
      }
      return at(matching_keys[0]);
    }

    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    template <typename T>
    cache_handle<Key, Value>
    cache_impl<Key, Value>::entry_for(handle const hint, T const& t) const
    {
      static_assert(detail::valid_supports_expression_v<Key, T>,
                    "The Key type does not provide a const-qualified 'supports' "
                    "function that takes an argument of the provided type.");
      if (hint and hint.key().supports(t)) {
        return hint;
      }

      return entry_for(t);
    }

    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    void
    cache_impl<Key, Value>::drop_unused()
    {
      drop_unused_but_last(0);
    }

    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    void
    cache_impl<Key, Value>::drop_unused_but_last(std::size_t const keep_last)
    {
      auto entries_to_drop = unused_entries_();
      // Sort in reverse-chronological order (according to sequence number).
      std::sort(begin(entries_to_drop),
                end(entries_to_drop),
                [](auto const& a, auto const& b) { return a.first > b.first; });

      if (std::size(entries_to_drop) <= keep_last) {
        return;
      }

      auto const erase_begin = cbegin(entries_to_drop) + keep_last;
      auto const erase_end = cend(entries_to_drop);
      for (auto it = erase_begin; it != erase_end; ++it) {
        // We need to protect access to the element that is about to
        // be erased (via entries_.find(...))--if we don't, then the
        // reference count can be incremented during an insert and we
        // end up erasing the element, creating invalid handles.
        accessor access_token;
        if (not entries_.find(access_token, it->second)) {
          continue;
        }

        // It's possible the reference count to the element was
        // increased between the unused_entries_() call and the
        // entries_.find(...) call made directly above.  We therefore
        // check that the reference count is actually zero before
        // erasing the element.
        if (access_token->second.reference_count() != 0u) {
          continue;
        }

        entries_.erase(access_token);
      }
    }

    #if CET_CONCEPTS_AVAILABLE
    template <::hep::concurrency::detail::hashable_cache_key Key, typename Value>
    #else
    template <typename Key, typename Value>
    #endif
    void
    cache_impl<Key, Value>::shrink_to_fit()
    {
      HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD();
      drop_unused();
      std::vector<count_value_type> used_keys;
      std::transform(begin(entries_),
                     end(entries_),
                     back_inserter(used_keys),
                     [](auto const& pr) {
                       return count_value_type{pr.first, pr.second.count_};
                     });
      counts_ = count_map_t(begin(used_keys), end(used_keys));
    }
  }
}

#endif /* hep_concurrency_cache_h */

// Local Variables:
// mode: c++
// End:
