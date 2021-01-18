#ifndef FIOCCA_VIEW_EXT_HPP_
#define FIOCCA_VIEW_EXT_HPP_

#include <ranges>
#include <algorithm>

namespace std {

namespace ranges {

namespace ext {

template<class... Ts>
concept variadic_views = (view<Ts> && ...);
template<class... Ts>
concept variadic_sized_ranges = (sized_range<Ts> && ...);
template<class... Ts>
concept variadic_bidirectional_ranges = (bidirectional_range<Ts> && ...);

// Swap the order of template arguments.
// The motivation is just to apply this concept to the
// restrictions specified in the later defined concepts.
template<typename I, typename S>
concept is_sentinel = sentinel_for<S, I>;
template<typename S>
concept traceable_sentinel = requires(S sentinel) {
  { sentinel.prev() } -> is_sentinel<S>;
};
template<typename T>
concept prev_traceable =
    bidirectional_iterator<T> ||
    traceable_sentinel<T>;

template<typename Iterator>
requires prev_traceable<Iterator>
auto prev(Iterator iter) {
  if constexpr (std::bidirectional_iterator<Iterator>)
    return std::ranges::prev(iter);
  else /* Must specify prev member function */
    // For some underlying ranges, end() does not return the same
    // type as begin(), so --end() and prev(end()) do not compile.
    // This is a possible workaround for such sentinel type.
    return iter.prev();
}

template<typename Range>
auto back(Range&& r) { // The last element in the range.
  return std::ranges::ext::prev(end(forward<Range>(r)));
}
template<typename Range>
auto head(Range&& r) { // The head element prior to the beginning.
  return std::ranges::ext::prev(begin(forward<Range>(r)));
}

template<typename T>
struct const_trait { using type = const T; };
template<typename T>
struct const_trait<T&> {
  using type = const remove_reference_t<T>&;
};

template<typename T>
using const_trait_t = typename const_trait<T>::type;

namespace detail {

template<typename Integer, typename View>
requires integral<Integer>
constexpr auto min_size_impl(Integer base, View&& view) {
  if constexpr (sized_range<View>) {
    if constexpr (unsigned_integral<Integer>)
      return std::ranges::min(base, std::ranges::size(view));
    else
      return std::ranges::min(base, std::ranges::ssize(view));
  } else return base;
}

template<typename Integer, typename Head, typename... Views>
requires integral<Integer>
constexpr auto min_size_impl(Integer base, Head&& head, Views&&... views) {
  if constexpr (sized_range<Head>)
    if constexpr (unsigned_integral<Integer>)
      return min_size_impl( // Recursion based approach.
        std::ranges::min(base, std::ranges::size(forward<Head>(head))),
        forward<Views>(views)...);
    else
      return min_size_impl( // Recursion based approach.
        std::ranges::min(base, std::ranges::ssize(forward<Head>(head))),
        forward<Views>(views)...);
  else // Skip the infinite range (without size attribute).
    return min_size_impl(base, forward<Views>(views)...);
}

} // namespace detail

template<typename Head, typename... Views>
requires (sized_range<Head> || (sized_range<Views> || ...))
constexpr auto min_size(Head&& head, Views&&... views) {
  if constexpr (sized_range<Head>)
    return detail::min_size_impl(
      std::ranges::size(forward<Head>(head)),
      forward<Views>(views)...);
  else return min_size(forward<Views>(views)...);
}
template<typename Head, typename... Views>
requires (sized_range<Head> || (sized_range<Views> || ...))
constexpr auto min_ssize(Head&& head, Views&&... views) {
  if constexpr (sized_range<Head>)
    return detail::min_size_impl(
      std::ranges::ssize(forward<Head>(head)),
      forward<Views>(views)...);
  else return min_ssize(forward<Views>(views)...);
}

} // namespace ext

} // namespace ranges

// Extensions to standard library.
namespace ext = ranges::ext;

} // namespace std

#endif // FIOCCA_VIEW_EXT_HPP_
