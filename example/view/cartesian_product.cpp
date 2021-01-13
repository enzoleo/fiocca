#include <iostream>
#include <array>
#include <vector>
#include "view/views.hpp"

auto main() -> int {
  std::array vec1 {'a', 'b', 'c', 'd', 'e'};
  std::vector vec2 {'w', 'x', 'y', 'z'};

  auto cp_view = std::views::cartesian_product(
    vec1 | std::views::all,
    vec2 | std::views::all);

  for (const auto& [x, y]: cp_view | std::views::reverse) {
    std::cout << x << y << "\n";
  }
  for (auto it = cp_view.cbegin(); it != cp_view.cend(); ++it) {
    std::cout << std::get<0>(*it) << " " << std::get<1>(*it) << std::endl;
  }

  return 0;
}
