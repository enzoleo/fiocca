#ifndef FIOCCA_UTILITY_HPP_
#define FIOCCA_UTILITY_HPP_

#include <type_traits>

namespace fiocca {

namespace detail {

template<typename Array, class Tuple, std::size_t index>
struct fill_array_helper {
  static_assert(index < std::tuple_size<Array>::value);
  static void fill(const Tuple& tup, Array& arr) {
    arr[index] = std::get<index>(tup);
    fill_array_helper<Array, Tuple, index - 1>::fill(tup, arr);
  }
};
template<typename Array, typename Tuple>
struct fill_array_helper<Array, Tuple, 0> {
  static_assert(0 < std::tuple_size<Array>::value, "invalid array index.");
  static void fill(const Tuple& tup, Array& arr) {
    arr[0] = std::get<0>(tup);
  }
};

} // namespace detail

#ifdef __cpp_concepts
template<typename T>
concept Floating = std::is_floating_point<T>::value;
#endif

template<
  typename Tuple, typename DataType = std::tuple_element_t<0, Tuple>,
  typename ArrayType = std::array<DataType, std::tuple_size<Tuple>::value>
  >
constexpr auto fill_array(const Tuple& tup) {
  ArrayType arr { };
  detail::fill_array_helper<
    ArrayType, Tuple,
    std::tuple_size<ArrayType>::value - 1>::fill(tup, arr);
  return arr;
}

} // namespace fiocca

#endif // FIOCCA_UTILITY_HPP_
