
#ifndef FIOCCA_POINT_HPP_
#define FIOCCA_POINT_HPP_

#include <cmath>
#include <array>
#include <type_traits>
#include <functional>

namespace fiocca {

/**
 * @brief Two-dimensional point struct.
 * Note that this struct is hightly public, roughly a wrapper of a pair.
 * You are allowed to modify coordinates freely outside the struct.
 * Structured binding are automatically enabled.
 */
template<class DataType>
struct Point {  
  using type = DataType;
  // The dimension of planar point defaults to be 2.
  static constexpr std::size_t dim() { return 2; }

  // Convert to a standard pair/tuple struct.
  constexpr auto pair() const { return std::make_pair(x, y); }

  // Whether the current point can dominate another point.
  // Here `dominate` means that all components are larger.
  constexpr auto dominate(const Point& p) {
    return this->x >= p.x && this->y >= p.y;
  }

  constexpr auto& operator[](std::size_t index) {
    // Probably this overloading requires assert to check the input
    // index is only 0 or 1. Here we still allow any non-negative inputs.
    // assert(index == 0 || index == 1);
    return index? y : x;
  }
  constexpr const auto& operator[](std::size_t index) const {
    // Probably this overloading requires assert to check the input
    // index is only 0 or 1. Here we still allow any non-negative inputs.
    // assert(index == 0 || index == 1);
    return index? y : x;
  }

  // Two-dimensional coordinates.
  DataType x, y;
};

// Operator overloading (arithmetic operations).
template<class DataType>
inline constexpr auto
operator+(const Point<DataType>& lhs, const Point<DataType>& rhs) {
  return Point<DataType> { lhs.x + rhs.x, lhs.y + rhs.y };
}
template<class DataType>
inline constexpr auto
operator-(const Point<DataType>& lhs, const Point<DataType>& rhs) {
  return Point<DataType> { lhs.x - rhs.x, lhs.y - rhs.y };
}
template<class DataType>
inline constexpr auto
operator+=(Point<DataType>& lhs, const Point<DataType>& rhs) {
  lhs.x += rhs.x, lhs.y += rhs.y;
}
template<class DataType>
inline constexpr auto
operator-=(Point<DataType>& lhs, const Point<DataType>& rhs) {
  lhs.x -= rhs.x, lhs.y -= rhs.y;
}

// Whether a point dominate another point.
// Here `dominate` means that all components are larger.
template<class DataType>
inline constexpr auto
dominate(const Point<DataType>& lhs, const Point<DataType>& rhs) {
  return lhs.x >= rhs.x && lhs.y >= rhs.y;
}

// Template type alias.
using Point2b = Point<unsigned char>;
using Point2s = Point<short>;
using Point2w = Point<unsigned short>;
using Point2i = Point<int>;
using Point2u = Point<unsigned int>;
using Point2f = Point<float>;
using Point2d = Point<double>;

} // namespace fiocca

#endif // FIOCCA_POINT_HPP_
