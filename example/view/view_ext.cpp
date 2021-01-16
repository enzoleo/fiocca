#include <iostream>
#include <array>
#include <vector>
#include "view/views.hpp"

auto main() -> int {
  // The initialized STL containers.
  std::array arr1 { 'a', 'b', 'c', 'd', 'e' };
  std::vector vec2 { 'w', 'x', 'y', 'z' };

  // Construct a cartesian_product of two views.
  auto cp_view = std::views::cartesian_product(
    arr1 | std::views::all,
    vec2 | std::views::all);

  for (const auto& [x, y]: cp_view | std::views::reverse) {
    (void) x; (void) y;
    // Uncomment the following line to print values on the screen.
    // std::cout << x << y << "\n";
  }

  // This is actually a decomposition of tuple. The range iterator is
  // dereferenced and then generate a tuple. The tuple itself can be a
  // non-reference type, but the inner element types always prefer
  // references.
  //     (1) auto   [x, y] : cp_view { }
  //     (2) auto&  [x, y] : cp_view { }
  //     (3) auto&& [x, y] : cp_view { }
  // The above three structured bindings perform in different ways. Pay
  // attention to the inner type, it can be reference even if in (1).
  for (auto [x, y] : cp_view | std::views::take(15)) {
    // The following line is allowed! The tuple element type is reference.
    // x = 'x';
    (void) x; (void) y;
    // Although the tuple type is non-reference, the elements inside the
    // tuple actually have reference types.
  }

  for (auto it = cp_view.cbegin(); it != cp_view.cend(); ++it) {
    (void) it;
    // Uncomment the following line for value print.
    // It is equivalent to the range-based for loop.
    // std::cout << std::get<0>(*it) << " " << std::get<1>(*it) << std::endl;
 
    // The following line is illegal <- assignment of read-only location.
    // The dereference type of const iterator is/prefers const reference.  
    // std::get<0>(*it) = 'x';
  }

  for (auto [x, y] : cp_view | std::views::cycle | std::views::take(30)) {
    (void) x; (void) y;
    // Uncomment the following line to print the cartesian product values
    // defined above in a cycle without an end.
    // std::cout << x << " " << y << std::endl;
  }

  // Construct a zip of two views.
  auto zip_view = std::views::zip(
    arr1 | std::views::all | std::views::cycle,
    vec2 | std::views::all | std::views::cycle);

  for (auto&& [x, y]: zip_view | std::views::take(30)) {
    (void) x; (void) y;
    // Uncomment the following line to print values on the screen.
    // std::cout << x << y << "\n";
  }

  return 0;
}