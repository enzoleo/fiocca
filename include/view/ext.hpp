#ifndef FIOCCA_VIEW_EXT_HPP_
#define FIOCCA_VIEW_EXT_HPP_

#include <ranges>

namespace std {

namespace ranges {

template<class... Ts>
concept variadic_views = (std::ranges::view<Ts> && ...);
template<class... Ts>
concept variadic_sized_ranges = (std::ranges::sized_range<Ts> && ...);

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_EXT_HPP_
