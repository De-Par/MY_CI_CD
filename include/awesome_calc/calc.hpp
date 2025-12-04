#pragma once

#include <stdexcept>
#include <vector>
#include <cstddef>

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

// Взвешенное среднее; проверяет размер векторов и сумму весов.
AWESOME_CALC_API double weighted_mean(const std::vector<double> &values,
                                      const std::vector<double> &weights);

// Медиана; выбрасывает std::invalid_argument при пустом векторе.
AWESOME_CALC_API double median(std::vector<double> values);

// Сатурированное сложение с границами [min_value, max_value].
AWESOME_CALC_API int clamp_add(int a, int b, int min_value, int max_value);

// Накопитель статистики для демонстрации stateful-API и тестов фикстурой.
class AWESOME_CALC_API RunningStats {
public:
    RunningStats() = default;

    // Добавляет значение в поток.
    void push(double value);

    // Сбрасывает накопленное состояние.
    void reset();

    // Количество элементов в накопителе.
    std::size_t count() const;

    // Сумма элементов; 0.0 для пустого состояния.
    double sum() const;

    // Среднее; выбрасывает std::invalid_argument, если нет данных.
    double average() const;

    // Минимум/максимум; выбрасывают std::invalid_argument для пустого состояния.
    double min() const;
    double max() const;

private:
    double sum_ = 0.0;
    double min_ = 0.0;
    double max_ = 0.0;
    std::size_t count_ = 0;
};

} // namespace awesome_calc
