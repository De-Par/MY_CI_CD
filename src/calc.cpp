#include "awesome_calc/calc.hpp"

namespace awesome_calc {
int add(int a, int b) {
    return a + b;
}

double mean(const std::vector<double> &values) {
    if (values.empty()) {
        // Явно сигнализируем об ошибочных входных данных.
        throw std::invalid_argument("values must not be empty");
    }

    double sum = 0.0; // Накопление суммы в double, чтобы избежать целочисленного деления.
    for (double v : values) {
        sum += v;
    }
    return sum / static_cast<double>(values.size());
}

} // namespace awesome_calc
