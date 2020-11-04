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
  template <typename T>
  struct returns_bool : std::false_type {};

  template <typename T, typename... Args>
  struct returns_bool<bool (T::*)(Args...) const> : std::true_type {};

  template <typename T, typename... Args>
  struct returns_bool<bool (T::*)(Args...) const noexcept> : std::true_type {};

  template <typename T, typename = void>
  struct has_supports : std::false_type {};

  template <typename T>
  struct has_supports<T, std::void_t<decltype(&T::supports)>> : std::true_type {
    static_assert(returns_bool<decltype(&T::supports)>::value,
                  "The 'supports' function must be const-qualified and return "
                  "a boolean value.");
  };

  template <typename T>
  constexpr bool has_supports_fcn_v = has_supports<T>::value;
}

#endif /* hep_concurrency_detail_cache_key_supports_h */

// Local Variables:
// mode: c++
// End:
