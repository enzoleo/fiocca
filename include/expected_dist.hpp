#ifndef FIOCCA_EXPECTED_DIST_HPP_
#define FIOCCA_EXPECTED_DIST_HPP_

#include "rect.hpp"
#include "numeric_integral.hpp"

namespace fiocca {

/**
 * @brief Calculate the expected distance between two rectangles. The
 *  return type is deduced by the compiler.
 * @tparam DataType the datatype of coordinates. It MUST be floating
 *  types to enable computation. However, the constraint will be set
 *  in implementations instead.
 * @param lhs the lefthand side rectangle.
 * @param rhs the righthand side rectangle.
 * @return Floating (should be @DataType) representing distance.
 */
template<class DataType>
auto expected_dist(const Rect<DataType>& lhs, const Rect<DataType>& rhs);

/**
 * @brief A system consisting of two rectangles.
 * The constructors are declared private so it is invisible outside class.
 * Only friend function is allowed to access it, for distance calculation.
 */
#ifdef __cpp_concepts
template<class DataType = double>
requires Floating<DataType>
#else
template<
  class DataType = double,
  typename Enable = std::enable_if_t<
    std::is_floating_point<DataType>::value>
  >
#endif
class TwinRect {
public:
  /**
   * @brief This function computes the average distance between two points
   *  randomly selected from the two rectangles. Specifically, it calculates
   *  a tedious integral:
   *      I = \int_{(x1, y1) \in [a, b]} \int_{(x2, y2) \in [c, d]}
   *            \sqrt{(x1 - x2) ^ 2 + (y1 - y2) ^ 2}
   *          d[x1] d[x2] d[y1] d[y2]
   * Monte-Carlo approach is feasible, however, much more imprecise. Numeric
   * integrals are also feasible that is capable of attaining a good
   * accuracy. But it might spend too much time. We use an explicit closed
   * representation to compute the target integral. 
   */
  auto dist() -> DataType {
    DataType result = 0;
    // [CASE 1] the two rectangles both have positive width.
    // Call the most complicated integral calculator.
    // They are allowed to degraded into horizontal lines.
    if (rect1_.w() && rect2_.w())
      result = _int_xdens(coord0[0], coord0[1])
             - _int_xdens(coord0[2], coord0[3])
             + _int_dens(coord0[1], coord0[2]) * (coord0[1] - coord0[0])
             - _int_dens(coord0[0], coord0[1]) * coord0[0]
             + _int_dens(coord0[2], coord0[3]) * coord0[3];
    // [CASE 2] the two rectangles are both degraded to vertical lines.
    // They are allowed to be a point.
    else if (!rect1_.w() && !rect2_.w())
      result = dens(delta1);
    else // [CASE 3] verticle line to rectangle/horizontal line.
      result = _int_dens(coord0[0], coord0[3]);
    return result / factor;
  }

  // Friend function declaration.
  // Note that the constructor of this class are delacred private.
  friend auto expected_dist<>(const Rect<DataType>& lhs,
                              const Rect<DataType>& rhs);

private:
  // Default PRIVATE constructor with two rectangles.
  TwinRect(const Rect<DataType>& rect1, const Rect<DataType>& rect2)
      : rect1_(rect1), rect2_(rect2) {
    auto [ w1, h1 ] = rect1_.shape();
    auto [ w2, h2 ] = rect2_.shape();
    delta1 = rect2_.x1() - rect1_.x1();
    delta2 = rect2_.y1() - rect1_.y1();
    factor = (w1? w1 : 1) * (h1? h1 : 1) * (w2? w2 : 1) * (h2? h2 : 1);

    // Structured binding and reassignment.
    auto [ lb1, ub1 ] = std::minmax({ 0., w2 - w1 });
    auto [ lb2, ub2 ] = std::minmax({ 0., h2 - h1 });
    coord0 = { delta1 - w1, delta1 + lb1, delta1 + ub1, delta1 + w2 };
    coord1 = { delta2 - h1, delta2 + lb2, delta2 + ub2, delta2 + h2 };
  }

  static auto f(DataType p, DataType q, DataType x) -> DataType {
    DataType xsq = x * x;
    DataType psqs = p * p + xsq, qsqs = q * q + xsq;
    DataType result = qsqs * std::sqrt(qsqs) - psqs * std::sqrt(psqs);
    return result / 3.;
  }
    
  static auto g(DataType p, DataType q, DataType x) -> DataType {
    DataType psq = p * p, qsq = q * q, xsq = x * x;
    DataType prt = std::sqrt(psq + xsq), qrt = std::sqrt(qsq + xsq);
    DataType result = q * qrt - p * prt
        + (x? xsq * std::log((q + qrt) / (p + prt)) : 0) / 2.;
    return result / 2.;
  }

  auto dens(DataType x) const -> DataType {
    if (rect1_.h() && rect2_.h())
      return f(coord1[0], coord1[1], x)
           - f(coord1[2], coord1[3], x)
           - g(coord1[0], coord1[1], x) * coord1[0]
           + g(coord1[1], coord1[2], x) * (coord1[1] - coord1[0])
           + g(coord1[2], coord1[3], x) * coord1[3];
    else if (!rect1_.h() && !rect2_.h())
      return std::sqrt(x * x + delta2 * delta2);
    return g(coord1[0], coord1[3], x);
  }

  static auto _idfint_f(DataType p, DataType q, DataType x) -> DataType {
    DataType psq = p * p, qsq = q * q, xsq = x * x;
    DataType prt = std::sqrt(psq + xsq), qrt = std::sqrt(qsq + xsq);
    DataType result;
    result = x * (2 * xsq + 5 * qsq) * qrt
           - x * (2 * xsq + 5 * psq) * prt
           + (q? 3. * qsq * qsq * std::log(x + qrt) : 0)
           - (p? 3. * psq * psq * std::log(x + prt) : 0);
    return result / 24.;
  }

  static auto _idfint_g(DataType p, DataType q, DataType x) -> DataType {
    DataType psq = p * p, qsq = q * q, xsq = x * x;
    DataType prt = std::sqrt(psq + xsq), qrt = std::sqrt(qsq + xsq);
    DataType result;
    //DataType tmp = almost_zero(xsq)? 
    result = 2. * x * (q * qrt - p * prt)
           + (q? q * qsq * std::log(x + qrt) : 0)
           - (p? p * psq * std::log(x + prt) : 0)
           + (x? x * xsq * std::log((q + qrt) / (p + prt)) : 0);
#if 1
    if (x && almost_zero(xsq)) {
      std::cout << result << " " << prt << std::endl;
    }
#endif
    return result / 6.;
  }

  static auto _idfint_xf(DataType p, DataType q, DataType x) -> DataType {
    DataType psps = p * p + x * x, qsqs = q * q + x * x;
    DataType result = qsqs * qsqs * std::sqrt(qsqs)
                    - psps * psps * std::sqrt(psps);
    return result / 15.;
  }

  static auto _idfint_xg(DataType p, DataType q, DataType x) -> DataType {
    DataType psq = p * p, qsq = q * q, xsq = x * x;
    DataType prt = std::sqrt(psq + xsq);
    DataType qrt = std::sqrt(qsq + xsq);
    DataType result;
    result = 2. * q * (5 * xsq + 2 * qsq) * qrt
           - 2. * p * (5 * xsq + 2 * psq) * prt
           + 6. * (x? xsq * xsq * std::log((q + qrt) / (p + prt)) : 0);
    return result / 48.;
  }

  DataType _int_f(DataType p, DataType q, DataType lower, DataType upper) {
    return _idfint_f(p, q, upper) - _idfint_f(p, q, lower);
  }

  DataType _int_g(DataType p, DataType q, DataType lower, DataType upper) {
    return _idfint_g(p, q, upper) - _idfint_g(p, q, lower);
  }

  DataType _int_xf(DataType p, DataType q, DataType lower, DataType upper) {
    return _idfint_xf(p, q, upper) - _idfint_xf(p, q, lower);
  }

  DataType _int_xg(DataType p, DataType q, DataType lower, DataType upper) {
    return _idfint_xg(p, q, upper) - _idfint_xg(p, q, lower);
  }

  DataType _int_dens(DataType lower, DataType upper) {
    if (rect1_.h() && rect2_.h())
      return _int_f(coord1[0], coord1[1], lower, upper)
           - _int_f(coord1[2], coord1[3], lower, upper)
           - _int_g(coord1[0], coord1[1], lower, upper) * coord1[0]
           + _int_g(coord1[1], coord1[2], lower, upper) * (coord1[1] - coord1[0])
           + _int_g(coord1[2], coord1[3], lower, upper) * coord1[3];
    else if (!rect1_.h() && !rect2_.h())
      return g(lower, upper, delta2);    
    return _int_g(coord1[0], coord1[3], lower, upper);
  }

  DataType _int_xdens(DataType lower, DataType upper) {
    if (rect1_.h() && rect2_.h())
      return _int_xf(coord1[0], coord1[1], lower, upper)
           - _int_xf(coord1[2], coord1[3], lower, upper)
           - _int_xg(coord1[0], coord1[1], lower, upper) * coord1[0]
           + _int_xg(coord1[1], coord1[2], lower, upper) * (coord1[1] - coord1[0])
           + _int_xg(coord1[2], coord1[3], lower, upper) * coord1[3];
    else if (!rect1_.h() && !rect2_.h())
      return f(lower, upper, delta2);
    return _int_xg(coord1[0], coord1[3], lower, upper);
  }
  
  // Two rectangles.
  Rect<DataType> rect1_, rect2_;
  
  // Position information.
  DataType delta1, delta2;

  // Area information.
  DataType factor;

  // Image information.
  std::array<DataType, 4> coord0, coord1;

};

// Implementation of distance calculator.
template<class DataType>
auto expected_dist(const Rect<DataType>& lhs, const Rect<DataType>& rhs) {
  TwinRect twin_rect(lhs, rhs);
  return twin_rect.dist();
}

} // namespace fiocca

#endif // FIOCCA_EXPECTED_DIST_HPP_
