#pragma once

#include <stdexcept>
#include <vector>

namespace awesome_calc {

// Складывает два целых числа без побочных эффектов.
int add(int a, int b);

// Среднее арифметическое; бросает std::invalid_argument при пустом векторе.
double mean(const std::vector<double>& values);

} // namespace awesome_calc
