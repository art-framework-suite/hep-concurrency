#ifndef hep_concurrency_detail_cache_entry_h
#define hep_concurrency_detail_cache_entry_h

// ===================================================================
// The cache_entry class is a reference-counted object that is used as
// the value_type of the cache.  For more details, see notes in
// cache.h
//
// N.B. This is not intended to be user-facing.
// ===================================================================

#include "cetlib_except/exception.h"

#include <atomic>
#include <memory>

namespace hep::concurrency {
  template <typename Key, typename Value>
  class cache;
}

namespace hep::concurrency::detail {
  struct entry_count {
    entry_count(std::size_t id, unsigned int n)
      : sequence_number{id}, use_count{n}
    {}
    std::size_t sequence_number;
    std::atomic<unsigned int> use_count;
  };

  using entry_count_ptr = std::shared_ptr<entry_count>;

  inline auto
  make_counter(std::size_t const sequence_number, unsigned int offset = 0)
  {
    return std::make_shared<entry_count>(sequence_number, offset);
  }
  inline auto
  make_invalid_counter()
  {
    return make_counter(-1ull, -1u);
  }

  // -------------------------------------------------------------------
  template <typename T>
  class cache_entry {
  public:
    cache_entry() = default;

    template <typename U = T>
    cache_entry(U&& u, entry_count_ptr counter)
      : value_{std::make_unique<T>(std::forward<U>(u))}
      , count_{std::move(counter)}
    {}

    T const&
    get() const
    {
      if (value_.get() == nullptr) {
        throw cet::exception("Invalid cache entry dereference.")
          << "Cache entry " << count_->sequence_number << " is empty.";
      }
      return *value_;
    }

    void
    increment_reference_count()
    {
      ++count_->use_count;
    }
    void
    decrement_reference_count()
    {
      --count_->use_count;
    }

    std::size_t
    sequence_number() const noexcept
    {
      return count_->sequence_number;
    }

    unsigned int
    reference_count() const noexcept
    {
      return count_->use_count;
    }

    template <typename Key, typename Value>
    friend class ::hep::concurrency::cache;

  private:
    std::unique_ptr<T> value_{nullptr};
    entry_count_ptr count_{make_invalid_counter()};
  };
}

#endif /* hep_concurrency_detail_cache_entry_h */

// Local Variables:
// mode: c++
// End:
