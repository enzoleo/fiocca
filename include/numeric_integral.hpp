#ifndef FIOCCA_NUMERIC_INTEGRAL_HPP_
#define FIOCCA_NUMERIC_INTEGRAL_HPP_

#include <iostream>
#include <vector>
#include "utility.hpp"

namespace fiocca {

namespace integral {

template<typename T, typename ValueType>
concept Integrable = Floating<ValueType> && requires(T f, ValueType x) {
  { f(x) } -> Floating;
};

/**
 * @brief The classical trapezoid algorithm.
 * @param integrand the function to be integrated. It should satisfy
 *  specific constraint that function call `integrand(floating) ->
 *  floating` must be legal.
 * @param min the lower bound of integral interval.
 * @param max the upper bound of integral interval.
 * @param ngrid the number of grids to divide the interval, which
 *  determines the precision (cannot be specified explicitly).
 * @return the integral result.
 */
template<class DataType, class Integrand>
requires Integrable<Integrand, DataType>
constexpr auto trapezoid(Integrand&& integrand,
                         DataType min, DataType max,
                         size_t ngrid = 1e+6) {
  DataType sum = 0; 
  DataType delta = (max - min) / ngrid;
#pragma omp parallel for reduction (+:sum)
  for (size_t i = 0; i != ngrid; ++i)
    sum += integrand(min + i * delta) * delta;
  return sum;
}

/**
 * @brief The classical Simpson algorithm.
 * @param integrand the function to be integrated. It should satisfy
 *  specific constraint that function call `integrand(floating) ->
 *  floating` must be legal.
 * @param min the lower bound of integral interval.
 * @param max the upper bound of integral interval.
 * @param ngrid the number of grids to divide the interval, which
 *  determines the precision (cannot be specified explicitly).
 * @return the integral result.
 */
template<class DataType, class Integrand>
requires Integrable<Integrand, DataType>
constexpr auto simpson(Integrand&& integrand,
                       DataType min, DataType max,
                       size_t ngrid = 1e+6) {
  DataType sum = 0;
  DataType delta = (max - min) / ngrid;
#pragma omp parallel for reduction (+:sum)
  for (size_t i = 1; i != ngrid; ++i)
    if (i & 1) sum += 4 * integrand(min + i * delta);
    else       sum += 2 * integrand(min + i * delta);       
  sum += integrand(min) + integrand(max);
  sum *= delta / 3;
  return sum;
}

/**
 * @brief The classical Romberg algorithm.
 * @param integrand the function to be integrated. It should satisfy
 *  specific constraint that function call `integrand(floating) ->
 *  floating` must be legal.
 * @param min the lower bound of integral interval.
 * @param max the upper bound of integral interval.
 * @param accuracy the accuracy to (probably) early stop the process.
 * @param max_steps the maximal number of steps to perform transformation,
 *  which also determines the precision.
 * @return the integral result.
 */
template<class DataType, class Integrand>
requires Integrable<Integrand, DataType>
auto romberg(Integrand&& integrand,
             DataType min, DataType max,
             DataType accuracy = (DataType)1e-11,
             size_t max_steps = 1e+2) {
  // Buffers for initialization of romberg series.
  std::vector<DataType> pre_row(max_steps), cur_row(max_steps);
  DataType h = max - min;
    
  // First trapezoidal step.
  pre_row[0] = (integrand(min) + integrand(max)) * h * 0.5;
  for (size_t i = 1; i != max_steps; ++i) {
    h /= 2.0;
    DataType sum = 0;
    size_t iteration = 1 << (i - 1);
    // WARNING: the step here is possible to be very slow!
    // If your `accuracy' is very small, the variable @iteration
    // will possibly be relatively big in some cases, which causes
    // low speed during the entire integration.
    for (size_t j = 0; j < iteration; ++j)
      sum += integrand(min + (2 * j + 1) * h);
        
    cur_row[0] = h * sum + 0.5 * pre_row[0];
    for (size_t j = 0; j < i; ++j) {
      DataType factor = std::pow(4, j + 1) - 1;
      cur_row[j + 1] = cur_row[j] + (cur_row[j] - pre_row[j]) / factor;
    }
    // Return the result if the accuracy is qualified.
    if (i > 1 && std::fabs(pre_row[i - 1] - cur_row[i]) < accuracy)
      return cur_row[i - 1];

    // Swap previous and current rows as we only need the last row.
    // Note that this is an O(1) operation.
    std::swap(pre_row, cur_row);
  }
  return pre_row.back();
}

} // namespace integral

} // namespace fiocca

#endif // FIOCCA_NUMERIC_INTEGRAL_HPP_
