#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <array>
#include "rect.hpp"
#include "expected_dist.hpp"
using namespace fiocca;

auto main() -> int {
  int repeat;
  std::cin >> repeat;
  for (int i = 0; i < repeat; ++i) {
    double x1, x2, x3, x4, y1, y2, y3, y4;
    std::cin >> x1 >> x2 >> y1 >> y2 >> x3 >> x4 >> y3 >> y4;
    Rect<double> rect1(x1, y1, x2, y2);
    Rect<double> rect2(x3, y3, x4, y4);
    auto t1 = std::chrono::steady_clock::now();
    for (int j = 0; j < 1; ++j) expected_dist(rect1, rect2);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << std::setprecision(3)
              << std::chrono::duration<double, std::milli>(t2 - t1).count()
              << "ms:\t" << std::setprecision(52)
              << expected_dist(rect1, rect2) << std::endl;
  }

  std::cout << std::setprecision(6);
  Rect rect(0.2, 0.3, 5.6, 5.8);
  Point2d p { 2.2, 6.3 };
  std::cout << rect.contain(p) << std::endl;
  
  auto res = integral::trapezoid(
    [](double x) { return x * x; },
    { { 0.0, 1.0} }, { 10000 }
  );
  std::cout << res << std::endl;

  return 0;
}
