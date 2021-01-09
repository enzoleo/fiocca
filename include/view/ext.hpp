#ifndef FIOCCA_VIEW_EXT_HPP_
#define FIOCCA_VIEW_EXT_HPP_

#include <ranges>

namespace std {

namespace ranges {

template<class... Ts>
concept variadic_views = (view<Ts> && ...);
template<class... Ts>
concept variadic_sized_ranges = (sized_range<Ts> && ...);
template<class... Ts>
concept variadic_bidirectional_ranges = (bidirectional_range<Ts> && ...);

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_EXT_HPP_
