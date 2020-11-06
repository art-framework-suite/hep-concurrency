#ifndef hep_concurrency_detail_cache_key_supports_h
#define hep_concurrency_detail_cache_key_supports_h

// ========================================================================
// For some cases, the user will not know what the key is.  For
// example, if the key corresponds to an interval of validity
// represented by a pair of numbers [b, e), the user may want to
// retrieve the entry for some value that is supported by the range
// [b, e).
//
// The key_supports struct uses SFINAE to determine whether a 'key' of
// type K contains a member function that can be called via
// 'key.supports(t)`, where 't' is of type T.
// ========================================================================

#include <type_traits>

namespace hep::concurrency::detail {
  template <typename Key, typename T>
  using expression_t =
    decltype(std::declval<Key const>().supports(std::declval<T>()));

  template <typename Key, typename T, typename = void>
  struct valid_supports_expression : std::false_type {};

  template <typename Key, typename T>
  struct valid_supports_expression<Key, T, std::void_t<expression_t<Key, T>>>
    : std::true_type {};

  template <typename Key, typename T>
  constexpr bool valid_supports_expression_v =
    valid_supports_expression<Key, T>::value;
}

#endif /* hep_concurrency_detail_cache_key_supports_h */

// Local Variables:
// mode: c++
// End:
