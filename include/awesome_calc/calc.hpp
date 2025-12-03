#pragma once

#include <stdexcept>
#include <vector>

namespace awesome_calc {

#if defined(_WIN32) && defined(AWESOME_CALC_SHARED)
// На Windows для DLL требуется явный экспорт/импорт символов; на static/*nix макрос пустой.
#if defined(AWESOME_CALC_BUILD)
#define AWESOME_CALC_API __declspec(dllexport)
#else
#define AWESOME_CALC_API __declspec(dllimport)
#endif
#else
#define AWESOME_CALC_API
#endif

// Складывает два целых числа без побочных эффектов.
AWESOME_CALC_API int add(int a, int b);

// Среднее арифметическое; бросает std::invalid_argument при пустом векторе.
AWESOME_CALC_API double mean(const std::vector<double> &values);

} // namespace awesome_calc
