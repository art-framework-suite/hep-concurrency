#ifndef hep_concurrency_test_interval_of_validity_h
#define hep_concurrency_test_interval_of_validity_h

#include "tbb/concurrent_hash_map.h"

#include <utility>

namespace hep::concurrency::test {

  class interval_of_validity {
  public:
    using value_type = std::pair<unsigned int, unsigned int>;

    interval_of_validity(unsigned int begin, unsigned int end) noexcept
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
    operator==(interval_of_validity const& other) const noexcept
    {
      return range_ == other.range_;
    }

    std::size_t
    hash() const noexcept
    {
      return hasher_(range_.first) ^ hasher_(range_.second);
    }

    friend std::ostream& operator<<(std::ostream&,
                                    interval_of_validity const& iov);

  private:
    value_type range_;
    std::hash<unsigned> hasher_{};
  };

  inline std::ostream&
  operator<<(std::ostream& os, interval_of_validity const& iov)
  {
    return os << '[' << iov.range_.first << ", " << iov.range_.second << ')';
  }
}

#endif /* hep_concurrency_test_interval_of_validity_h */

// Local Variables:
// mode: c++
// End:
