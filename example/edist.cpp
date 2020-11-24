#include <iostream>
#include <iomanip>
#include <cmath>
#include "rect.hpp"
#include "expected_dist.hpp"

auto main() -> int {
  int repeat;
  std::cin >> repeat;
  std::cout << std::fixed << std::setprecision(14);
  for (int i = 0; i < repeat; ++i) {
    double x1, x2, x3, x4, y1, y2, y3, y4;
    std::cin >> x1 >> x2 >> y1 >> y2 >> x3 >> x4 >> y3 >> y4;
    fiocca::Rect<double> rect1(x1, y1, x2, y2);
    fiocca::Rect<double> rect2(x3, y3, x4, y4);
    fiocca::DoubleRect dr(rect1, rect2);
    std::cout << dr.dist() << std::endl;
  }

  return 0;
}
