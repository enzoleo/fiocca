#ifndef FIOCCA_RECT_HPP_
#define FIOCCA_RECT_HPP_

#include <iostream>
#include <iomanip>
#include <cmath>
#include "point.hpp"
#include "numeric_integral.hpp"

namespace fiocca {

/**
 * @brief A simple two-dimensional rectangle class.
 */
template<class DataType>
class Rect {
public:
  // Default constructor with no argument/four coordinates.
  constexpr Rect() : p1({ 0, 0 }), p2({ 0, 0 }) { }
  constexpr Rect(DataType x1_, DataType x2_, DataType y1_, DataType y2_)
      : p1({ std::min(x1_, x2_), std::max(x1_, x2_) }),
        p2({ std::min(y1_, y2_), std::max(y1_, y2_) }) {
    // To simplify the calculation process and avoid mistakes,
    // here we need to make sure x1 <= x2 and y1 <= y2.
  }

  // Construct rectangle from bottom left and top right points.
  constexpr Rect(const Point<DataType>& p1_, const Point<DataType>& p2_) {
    // Check range of input. Swap coordinates if illegal.
    p1 = { std::min(p1_.x, p2.x), std::min(p1_.y, p2_.y) };
    p2 = { std::max(p1_.x, p2.x), std::max(p1_.y, p2_.y) };
  }

  // Attribute accessors/mutators.
  constexpr auto x1() const { return p1.x; }
  constexpr auto x2() const { return p2.x; }
  constexpr auto y1() const { return p1.y; }
  constexpr auto y2() const { return p2.y; }
  
  // Four corner points. Note that accessors of bottom left and top
  // right points return const references.
  constexpr const auto& bl() const { return p1; }
  constexpr const auto& tr() const { return p2; }
  constexpr auto br() const { return Point<DataType> { p2.x, p1.y }; }
  constexpr auto tl() const { return Point<DataType> { p1.x, p2.y }; }

  // Shape calculators.
  constexpr auto w() const { return p2.x - p1.x; }
  constexpr auto h() const { return p2.y - p1.y; }
  constexpr auto shape() const {
    return std::make_pair(p2.x - p1.x, p2.y - p1.y);
  }

  // Other geometric attributes.
  constexpr auto area() const { return w() * h(); }
  constexpr auto peri() const { return 2 * (w() + h()); }
  auto diam() const { return std::hypot(w(), h()); }

  // Whether a point is inside the rectangle.
  constexpr auto contain(Point<DataType>& p) const {
    return dominate(p, p1) && dominate(p2, p);
  }

protected:
  // Corner coordinates of the rectangle.
  // (x1, y2) -------------------- (x2, y2)
  //          |    Rectangle     |
  // (x1, y1) -------------------- (x2, y1)
  // Only need left bottom and top right points to represent it.
  Point<DataType> p1, p2;
};

} // namespace fiocca

#endif // FIOCCA_RECT_HPP_
