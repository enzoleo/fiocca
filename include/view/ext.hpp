#ifndef FIOCCA_VIEW_EXT_HPP_
#define FIOCCA_VIEW_EXT_HPP_

#include <ranges>

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
  return std::ranges::ext::prev(end(std::forward<Range>(r)));
}
template<typename Range>
auto head(Range&& r) { // The head element prior to the beginning.
  return std::ranges::ext::prev(begin(std::forward<Range>(r)));
}

} // namespace ext

} // namespace ranges

// Extensions to standard library.
namespace ext = ranges::ext;

} // namespace std

#endif // FIOCCA_VIEW_EXT_HPP_
