#ifndef hep_concurrency_cache_handle_h
#define hep_concurrency_cache_handle_h

// ====================================================================
// The cache handle is the user interface for accessing concurrent
// cache elements.  A handle that points to a specific cache element
// ensures that that element will not be deleted from the cache during
// the lifetime of the handle.
//
// A typical way of using the cache handle looks like:
//
//   cache<K, V> cache;
//   if (auto h = cache.entry_for(key)) {
//     auto const& value_for_key = *h;
//     h->some_member_function_of_type_V();
//     ...
//   }
//
// The above example demonstrates three aspects of handles:
//
//   - A valid/invalid handle is convertible to the Boolean values
//     true/false.
//   - Access to the underlying entry's immutable value is provided
//     via operator*.
//   - Access to the underlying entry's const-qualified member
//     functions is provided via operator->.
//
// N.B. A handle cannot in any way adjust the underlying value.  It is
//      considered immutable.
// ====================================================================

#include "cetlib_except/exception.h"
#include "hep_concurrency/detail/cache_entry.h"

#include <utility>

namespace hep::concurrency {

  template <typename Key, typename Value>
  class cache_handle {
  public:
    static constexpr cache_handle invalid() noexcept;
    explicit cache_handle(Key const* key, detail::cache_entry<Value>* entry);
    ~cache_handle();

    cache_handle(cache_handle const& other);
    cache_handle& operator=(cache_handle const& other);
    cache_handle(cache_handle&& other);
    cache_handle& operator=(cache_handle&& other);

    // Check whether handle points to valid cache entry
    bool is_valid() const noexcept;
    explicit operator bool() const noexcept { return is_valid(); }

    // General access
    Value const& operator*() const;
    Value const* operator->() const;
    Key const& key() const;
    std::size_t sequence_number() const;

    // Comparisons
    bool operator==(cache_handle other) const;
    bool operator!=(cache_handle other) const;

    // Remove access to cache entry
    void invalidate();

  private:
    constexpr cache_handle() = default;

    Key const* key_{nullptr};
    detail::cache_entry<Value>* entry_{nullptr};
  };

  // ----------------------------------------------------------------------------
  // Implementation below

  template <typename Key, typename Value>
  constexpr cache_handle<Key, Value>
  cache_handle<Key, Value>::invalid() noexcept
  {
    return {};
  }

  template <typename Key, typename Value>
  cache_handle<Key, Value>::cache_handle(Key const* key,
                                         detail::cache_entry<Value>* entry)
    : key_{key}, entry_{entry}
  {
    if (entry_) {
      entry_->increment_reference_count();
    }
  }

  template <typename Key, typename Value>
  cache_handle<Key, Value>::cache_handle(cache_handle const& other)
    : key_{other.key_}, entry_{other.entry_}
  {
    if (entry_) {
      entry_->increment_reference_count();
    }
  }

  template <typename Key, typename Value>
  cache_handle<Key, Value>&
  cache_handle<Key, Value>::operator=(cache_handle const& other)
  {
    if (other.entry_) {
      other.entry_->increment_reference_count();
    }

    invalidate();
    key_ = other.key_;
    entry_ = other.entry_;
    return *this;
  }

  template <typename Key, typename Value>
  cache_handle<Key, Value>::cache_handle(cache_handle&& other)
    : cache_handle{other} // Use copy c'tor
  {
    other.invalidate();
  }

  template <typename Key, typename Value>
  cache_handle<Key, Value>&
  cache_handle<Key, Value>::operator=(cache_handle&& other)
  {
    *this = other; // Use copy assignment operator
    other.invalidate();
    return *this;
  }

  template <typename Key, typename Value>
  bool
  cache_handle<Key, Value>::is_valid() const noexcept
  {
    return key_ != nullptr and entry_ != nullptr;
  }

  template <typename Key, typename Value>
  Value const& cache_handle<Key, Value>::operator*() const
  {
    if (entry_ == nullptr) {
      throw cet::exception("Invalid cache handle dereference.")
        << "Handle does not refer to any cache entry.";
    }
    return entry_->get();
  }

  template <typename Key, typename Value>
  Value const* cache_handle<Key, Value>::operator->() const
  {
    return &this->operator*();
  }

  template <typename Key, typename Value>
  Key const&
  cache_handle<Key, Value>::key() const
  {
    if (key_ == nullptr) {
      throw cet::exception("Invalid key access.")
        << "Handle does not refer to any cache entry.";
    }
    return *key_;
  }

  template <typename Key, typename Value>
  std::size_t
  cache_handle<Key, Value>::sequence_number() const
  {
    if (entry_ == nullptr) {
      throw cet::exception("Invalid sequence-number access.")
        << "Handle does not refer to any cache entry.";
    }
    return entry_->sequence_number();
  }

  template <typename Key, typename Value>
  bool
  cache_handle<Key, Value>::operator==(cache_handle const handle) const
  {
    if (this == &handle) {
      return true;
    }
    return key_ == handle.key_ and entry_ == handle.entry_;
  }

  template <typename Key, typename Value>
  bool
  cache_handle<Key, Value>::operator!=(cache_handle const handle) const
  {
    return not operator==(handle);
  }

  template <typename Key, typename Value>
  void
  cache_handle<Key, Value>::invalidate()
  {
    if (key_ != nullptr and entry_ != nullptr) {
      entry_->decrement_reference_count();
    }
    key_ = nullptr;
    entry_ = nullptr;
  }

  template <typename Key, typename Value>
  cache_handle<Key, Value>::~cache_handle()
  {
    invalidate();
  }

}

#endif /* hep_concurrency_cache_handle_h */

// Local Variables:
// mode: c++
// End:
