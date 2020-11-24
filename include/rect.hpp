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
  constexpr Rect() : p1({ 0, 0 }), p2({0, 0}) { }
  constexpr Rect(DataType _x1, DataType _x2, DataType _y1, DataType _y2)
      : p1({ std::min(_x1, _x2), std::max(_x1, _x2) }),
        p2({ std::min(_y1, _y2), std::max(_y1, _y2) }) {
    // To simplify the calculation process and avoid mistakes,
    // here we need to make sure x1 <= x2 and y1 <= y2.
  }

  // Construct rectangle from bottom left and top right points.
  constexpr Rect(const Point<DataType>& p1_, const Point<DataType>& p2_) {
    // Check range of input. Swap coordinates if illegal.
    p1 = { std::min(p1.x, p2.x), std::min(p1.y, p2.y) };
    p2 = { std::max(p1.x, p2.x), std::max(p1.y, p2.y) };
  }

  // Attribute accessors/mutators.
  constexpr auto x1() const { return p1.x; }
  constexpr auto x2() const { return p2.x; }
  constexpr auto y1() const { return p1.y; }
  constexpr auto y2() const { return p2.y; }
  
  // Bottom left point and top right point
  constexpr const auto& bl() const { return p1; }
  constexpr const auto& tr() const { return p2; }

  // Shape calculators.
  constexpr auto w() const { return p2.x - p1.x; }
  constexpr auto h() const { return p2.y - p1.y; }
  constexpr auto shape() const {
    return std::make_pair(p2.x - p1.x, p2.y - p1.y);
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
