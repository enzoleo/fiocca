#ifndef FIOCCA_UTILITY_HPP_
#define FIOCCA_UTILITY_HPP_

#include <type_traits>
#include <limits>
#include <array>

namespace fiocca {

namespace detail {

template<typename Array, class Tuple, std::size_t index>
struct fill_array_impl {
  static_assert(index < std::tuple_size<Array>::value);
  static void fill(const Tuple& tup, Array& arr) {
    arr[index] = std::get<index>(tup);
    fill_array_impl<Array, Tuple, index - 1>::fill(tup, arr);
  }
};
template<typename Array, typename Tuple>
struct fill_array_impl<Array, Tuple, 0> {
  static_assert(0 < std::tuple_size<Array>::value, "invalid array index.");
  static void fill(const Tuple& tup, Array& arr) {
    arr[0] = std::get<0>(tup);
  }
};

#ifndef __cpp_lib_to_array
template<class Type, std::size_t N, std::size_t... index>
constexpr std::array<std::remove_cv_t<Type>, N>
  to_array_impl(Type (&arr)[N], std::index_sequence<index...>) {
  return { { arr[index]... } };
}
template<class Type, std::size_t N, std::size_t... index>
constexpr std::array<std::remove_cv_t<Type>, N>
  to_array_impl(Type (&&arr)[N], std::index_sequence<index...>) {
  return { { std::move(arr[index])... } };
}
#endif

template<class ...Ts>
struct duplicator_impl {
  using child_type = duplicator_impl<
    std::tuple_element_t<0, std::tuple<Ts...> >, Ts...>;
  using tuple_type = std::tuple<Ts...>;
};

} // namespace detail

template<typename T>
concept Floating = std::is_floating_point<T>::value;

template<class... Ts>
concept Homogeneous =
    sizeof...(Ts) < 2 ||
    std::conjunction_v<
      std::is_same<std::tuple_element_t<0, std::tuple<Ts...> >, Ts>...
    >;

template<typename DataType>
requires Floating<DataType>
auto almost_zero(DataType x, size_t ulp = 1) {
  // The machine epsilon is applied. Refer to cppreference for more details.
  return std::fabs(x) <= std::numeric_limits<DataType>::epsilon() * ulp;
}

/**
 * @brief Convert a homogeneous tuple to array.
 *  For example, a tuple std::tuple<T, T, T, T> will be converted into
 *  an std::array<T, 4> automatically. 
 * 
 * @param tup the tuple to be converted. The type must satisfy the constraint
 *  that std::tuple_element_t<index, Tuple> for some indices is legal.
 * @return array type deduced automatically.
 */
template<
  typename Tuple, typename DataType = std::tuple_element_t<0, Tuple>,
  typename ArrayType = std::array<DataType, std::tuple_size<Tuple>::value>
  >
constexpr auto fill_array(const Tuple& tup) {
  ArrayType arr { };
  detail::fill_array_impl<
    ArrayType, Tuple,
    std::tuple_size<ArrayType>::value - 1>::fill(tup, arr);
  return arr;
}

#ifndef __cpp_lib_to_array
/**
 * @brief Creates a std::array from the one dimensional built-in array. This
 *  is a fall-back compatibility-aimed implementation according to the sample
 *  version of cppreference. In c++20, use std::to_array instead!
 * 
 * @tparam Type the homogeneous array datatype.
 * @tparam N the total size of the output array.
 * @return an std::array containing the same data as the input built-in array.
 */
template<class Type, std::size_t N>
constexpr std::array<std::remove_cv_t<Type>, N> to_array(Type (&arr)[N]) {
  return detail::to_array_impl(arr, std::make_index_sequence<N>{});
}
template<class Type, std::size_t N>
constexpr std::array<std::remove_cv_t<Type>, N> to_array(Type (&&arr)[N]) {
  return detail::to_array_impl(std::move(arr), std::make_index_sequence<N>{});
}
#endif

/**
 * @brief Duplicator that duplicate the template argument multiple times,
 *  according to the compile-time @dim argument.
 * 
 * @tparam dim the number of times to duplicate @T.
 * @tparam T the type to be duplicated.
 * 
 * duplicator<dim, T>::type is exactly duplicator_impl<T, ..., T> (@dim times).
 * Equivalently, call this type via duplicator_t<dim, T>. Besides of that,
 * duplicator<dim, T>::type::tuple_type is std::tuple<T, ..., T> (@dim times),
 * so the std::apply is allowed to invoke related functions with variadic
 * arguments in a smart way.
 */
template<std::size_t dim, class T>
struct duplicator { using type = duplicator<dim - 1, T>::type::child_type; };
template<class T>
struct duplicator<1, T> { using type = detail::duplicator_impl<T>; };
template<std::size_t dim, class T>
using duplicator_t = typename duplicator<dim, T>::type;
template<std::size_t dim, class T>
using duplicator_tuple_t = typename duplicator<dim, T>::type::tuple_type;

} // namespace fiocca

#endif // FIOCCA_UTILITY_HPP_
