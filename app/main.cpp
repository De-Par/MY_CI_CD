#include <iostream>
#include <vector>

#include "awesome_calc/calc.hpp"

int main() {
    // Мини-дэма: вызываем библиотеку и печатаем результаты.
    using namespace awesome_calc;

    int a = 2;
    int b = 3;

    std::cout << "add(" << a << ", " << b << ") = " << add(a, b) << "\n";

    std::vector<double> vals{1.0, 2.0, 3.0, 4.0};
    std::cout << "mean(1,2,3,4) = " << mean(vals) << "\n";

    return 0;
}
