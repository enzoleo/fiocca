#ifndef FIOCCA_VIEW_CYCLE_HPP_
#define FIOCCA_VIEW_CYCLE_HPP_

#include "ext.hpp"

namespace std {

namespace ranges {

template<input_range View>
class cycle_view : public view_interface<cycle_view<View> > {
public:
  cycle_view() = default;
  constexpr cycle_view(View&& view) : view_(forward<View>(view)) { }

private:
  View view_;
};

} // namespace ranges

} // namespace std

#endif // FIOCCA_VIEW_CYCLE_HPP_