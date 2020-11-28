
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

  // Convert to a standard pair/tuple struct.
  constexpr auto pair() const { return std::make_pair(x, y); }

  // Whether the current point can dominate another point.
  // Here `dominate` means that all components are larger.
  constexpr auto dominate(const Point& p) {
    return this->x >= p.x && this->y >= p.y;
  }

  // Two-dimensional coordinates.
  DataType x, y;
};

// Operator overloading (arithmetic operations).
template<class DataType>
inline constexpr auto operator+(const Point<DataType>& lhs,
                                const Point<DataType>& rhs) {
  return Point<DataType> { lhs[0] + rhs[0], lhs[1] + rhs[1] };
}
template<class DataType>
inline constexpr auto operator-(const Point<DataType>& lhs,
                                const Point<DataType>& rhs) {
  return Point<DataType> { lhs[0] - rhs[0], lhs[1] - rhs[1] };
}
template<class DataType>
inline constexpr auto operator+=(Point<DataType>& lhs,
                                 const Point<DataType>& rhs) {
  lhs[0] += rhs[0], lhs[1] += rhs[1];
}
template<class DataType>
inline constexpr auto operator-=(Point<DataType>& lhs,
                                 const Point<DataType>& rhs) {
  lhs[0] -= rhs[0], lhs[1] -= rhs[1];
}

// Whether a point dominate another point.
// Here `dominate` means that all components are larger.
template<class DataType>
inline constexpr auto dominate(const Point<DataType>& lhs,
                               const Point<DataType>& rhs) {
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

#endif // FIOCCA_POINT_HPP
