#ifndef EXPECTED_DIST_HPP_
#define EXPECTED_DIST_HPP_

#include "rect.hpp"
#include "numeric_integral.hpp"

namespace fiocca {

/* A system consisting of two rectangles */
//template<class Integral>
class DoubleRect {
public:
  /* Constructors */
  DoubleRect(const Rect<double>& _rectP, const Rect<double>& _rectQ)
      : rectP(_rectP), rectQ(_rectQ) {
    auto [ a, b ] = rectP.shape();
    auto [ c, d ] = rectQ.shape();
    delta1 = rectQ.x1() - rectP.x1();
    delta2 = rectQ.y1() - rectP.y1();
    factor = (a? a : 1) * (b? b : 1) * (c? c : 1) * (d? d : 1);

    coord0[0] = delta1 - a;
    coord0[1] = delta1 + std::min(0., c - a);
    coord0[2] = delta1 + std::max(0., c - a);
    coord0[3] = c + delta1;
        
    coord1[0] = delta2 - b;
    coord1[1] = delta2 + std::min(0., d - b);
    coord1[2] = delta2 + std::max(0., d - b);
    coord1[3] = d + delta2;
  }

  /**
   * This function computes the average distance between two points randomly
   * selected from the two rectangles.
   */
  auto dist() -> double {
    double result = 0;
    if (rectP.w() && rectQ.w())
      result = _int_xdens(coord0[0], coord0[1])
          - _int_xdens(coord0[2], coord0[3])
          + _int_dens(coord0[1], coord0[2]) * (coord0[1] - coord0[0])
          - _int_dens(coord0[0], coord0[1]) * coord0[0]
          + _int_dens(coord0[2], coord0[3]) * coord0[3];
    else if (!rectP.w() && !rectQ.w())
      result = dens(delta1);
    else
      result = _int_dens(coord0[0], coord0[3]);
    return result / factor;
  }

  //Integral integral;

private:

  double f(double p, double q, double x) {
    double xsq = x * x;
    double psqsum = p * p + xsq;
    double qsqsum = q * q + xsq;
    double result = qsqsum * std::sqrt(qsqsum)
        - psqsum * std::sqrt(psqsum);
    return result / 3.;
  }
    
  double g(double p, double q, double x) {
    double psq = p * p, qsq = q * q, xsq = x * x;
    double prt = std::sqrt(psq + xsq);
    double qrt = std::sqrt(qsq + xsq);
    double result = q * qrt - p * prt
        + (x? xsq * std::log((q + qrt) / (p + prt)) : 0);
    return result / 2.;
  }

  double dens(double x) {
    if (rectP.h() && rectQ.h())
      return f(coord1[0], coord1[1], x)
          - f(coord1[2], coord1[3], x)
          - g(coord1[0], coord1[1], x) * coord1[0]
          + g(coord1[1], coord1[2], x) * (coord1[1] - coord1[0])
          + g(coord1[2], coord1[3], x) * coord1[3];
    else if (!rectP.h() && !rectQ.h())
      return std::sqrt(x * x + delta2 * delta2);
    return g(coord1[0], coord1[3], x);
  }

  double _idfint_f(double p, double q, double x) {
    double psq = p * p, qsq = q * q, xsq = x * x;
    double prt = std::sqrt(psq + xsq);
    double qrt = std::sqrt(qsq + xsq);
    double result;
    result = x * (2 * xsq + 5 * qsq) * qrt
        - x * (2 * xsq + 5 * psq) * prt
        + (q? 3. * qsq * qsq * std::log(x + qrt) : 0)
        - (p? 3. * psq * psq * std::log(x + prt) : 0);
    return result / 24.;
  }

  double _idfint_g(double p, double q, double x) {
    double psq = p * p, qsq = q * q, xsq = x * x;
    double prt = std::sqrt(psq + xsq);
    double qrt = std::sqrt(qsq + xsq);
    double result;
    result = 2. * x * (q * qrt - p * prt)
        + (q? q * qsq * std::log(x + qrt) : 0)
        - (p? p * psq * std::log(x + prt) : 0)
        + (x? x * xsq * std::log((q + qrt) / (p + prt)) : 0);
    return result / 6.;
  }

  double _idfint_xf(double p, double q, double x) {
    double qsqsum = q * q + x * x;
    double pspsum = p * p + x * x;
    double result;
    result = qsqsum * qsqsum * std::sqrt(qsqsum)
        - pspsum * pspsum * std::sqrt(pspsum);
    return result / 15.;
  }

  double _idfint_xg(double p, double q, double x) {
    double psq = p * p, qsq = q * q, xsq = x * x;
    double prt = std::sqrt(psq + xsq);
    double qrt = std::sqrt(qsq + xsq);
    double result;
    result = 2. * q * (5 * xsq + 2 * qsq) * qrt
        - 2. * p * (5 * xsq + 2 * psq) * prt
        + 6. * (x? xsq * xsq * std::log((q + qrt) / (p + prt)) : 0);
    return result / 48.;
  }

  double _int_f(double p, double q, double lower, double upper) {
    return _idfint_f(p, q, upper) - _idfint_f(p, q, lower);
  }

  double _int_g(double p, double q, double lower, double upper) {
    return _idfint_g(p, q, upper) - _idfint_g(p, q, lower);
  }

  double _int_xf(double p, double q, double lower, double upper) {
    return _idfint_xf(p, q, upper) - _idfint_xf(p, q, lower);
  }

  double _int_xg(double p, double q, double lower, double upper) {
    return _idfint_xg(p, q, upper) - _idfint_xg(p, q, lower);
  }

  double _int_dens(double lower, double upper) {
    if (rectP.h() && rectQ.h())
      return _int_f(coord1[0], coord1[1], lower, upper)
          - _int_f(coord1[2], coord1[3], lower, upper)
          - _int_g(coord1[0], coord1[1], lower, upper) * coord1[0]
          + _int_g(coord1[1], coord1[2], lower, upper) * (coord1[1] - coord1[0])
          + _int_g(coord1[2], coord1[3], lower, upper) * coord1[3];
    else if (!rectP.h() && !rectQ.h())
      return g(lower, upper, delta2);    
    return _int_g(coord1[0], coord1[3], lower, upper);
  }

  double _int_xdens(double lower, double upper) {
    if (rectP.h() && rectQ.h())
      return _int_xf(coord1[0], coord1[1], lower, upper)
          - _int_xf(coord1[2], coord1[3], lower, upper)
          - _int_xg(coord1[0], coord1[1], lower, upper) * coord1[0]
          + _int_xg(coord1[1], coord1[2], lower, upper) * (coord1[1] - coord1[0])
          + _int_xg(coord1[2], coord1[3], lower, upper) * coord1[3];
    else if (!rectP.h() && !rectQ.h())
      return f(lower, upper, delta2);    
    return _int_xg(coord1[0], coord1[3], lower, upper);
  }
  
  /* Two rectangles */
  Rect<double> rectP, rectQ;
  
  /* Position information */
  double a, b, c, d;
  double delta1, delta2;

  /* Area information */
  double factor;

  /* Image information */
  std::array<double, 4> coord0, coord1;
};

} // namespace fiocca

#endif // EXPECTED_DIST_HPP_
