#include <iostream>
#include <vector>

#include "awesome_calc/calc.hpp"

int main() {
    // Демо: вызываем библиотеку, чтобы показать разные сценарии для тестов.
    using namespace awesome_calc;

    const int a = 2;
    const int b = 3;

    std::cout << "add(" << a << ", " << b << ") = " << add(a, b) << "\n";

    std::vector<double> vals{1.0, 2.0, 3.0, 4.0};
    std::cout << "mean(1,2,3,4) = " << mean(vals) << "\n";
    std::cout << "median(1,2,3,4) = " << median(vals) << "\n";

    std::vector<double> weights{0.1, 0.2, 0.3, 0.4};
    std::cout << "weighted_mean(1,2,3,4 | 0.1,0.2,0.3,0.4) = " << weighted_mean(vals, weights)
              << "\n";

    std::cout << "clamp_add(100, 50, -100, 120) = " << clamp_add(100, 50, -100, 120) << "\n";

    // Накопительная статистика — stateful API, удобно тестировать через фикстуры.
    RunningStats stats;
    stats.push(10.0);
    stats.push(20.0);
    stats.push(30.0);
    std::cout << "running_stats count=" << stats.count() << " sum=" << stats.sum()
              << " avg=" << stats.average() << " min=" << stats.min() << " max=" << stats.max()
              << "\n";

    return 0;
}
