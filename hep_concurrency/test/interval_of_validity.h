#ifndef hep_concurrency_test_interval_of_validity_h
#define hep_concurrency_test_interval_of_validity_h

#include "tbb/concurrent_hash_map.h"

#include <utility>

namespace hep::concurrency::test {

  class interval_of_validity {
  public:
    using value_type = std::pair<unsigned int, unsigned int>;

    interval_of_validity(unsigned int begin, unsigned int end)
      : range_{begin, end}
    {}

    bool
    supports(unsigned int const value) const noexcept
    {
      return range_.first <= value and value < range_.second;
    }

    bool
    supports(interval_of_validity const iov) const noexcept
    {
      return range_.first <= iov.range_.first and
             iov.range_.second <= range_.second;
    }

    bool
    operator<(interval_of_validity const& other) const noexcept
    {
      return range_ < other.range_;
    }

    bool
    operator==(interval_of_validity const& other) const noexcept
    {
      return range_ == other.range_;
    }

    friend class std::hash<interval_of_validity>;
    friend class tbb::tbb_hash_compare<interval_of_validity>;
    friend std::ostream& operator<<(std::ostream&,
                                    interval_of_validity const& iov);

  private:
    value_type range_;
  };

  inline std::ostream&
  operator<<(std::ostream& os, interval_of_validity const& iov)
  {
    return os << '[' << iov.range_.first << ", " << iov.range_.second << ')';
  }
}

namespace tbb {
  template <>
  struct tbb_hash_compare<hep::concurrency::test::interval_of_validity> {
    std::size_t
    hash(hep::concurrency::test::interval_of_validity const& iov) const
    {
      return hasher.hash(iov.range_);
    }

    bool
    equal(hep::concurrency::test::interval_of_validity const& lhs,
          hep::concurrency::test::interval_of_validity const& rhs) const
    {
      return hasher.equal(lhs.range_, rhs.range_);
    }

    tbb_hash_compare<hep::concurrency::test::interval_of_validity::value_type>
      hasher;
  };
}

namespace std {
  template <>
  struct hash<hep::concurrency::test::interval_of_validity> {
    std::size_t
    operator()(hep::concurrency::test::interval_of_validity const& iov) const
    {
      std::hash<unsigned> hash{};
      return hash(iov.range_.first) ^ hash(iov.range_.second);
    }
  };
}

#endif /* hep_concurrency_test_interval_of_validity_h */

// Local Variables:
// mode: c++
// End:
