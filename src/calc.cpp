#include "awesome_calc/calc.hpp"

#include <algorithm>

namespace awesome_calc {

int add(int a, int b) { return a + b; }

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

double weighted_mean(const std::vector<double> &values, const std::vector<double> &weights) {
    if (values.size() != weights.size()) {
        throw std::invalid_argument("values and weights must have the same size");
    }
    if (values.empty()) {
        throw std::invalid_argument("values must not be empty");
    }

    double wsum = 0.0;
    double acc = 0.0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        acc += values[i] * weights[i];
        wsum += weights[i];
    }
    if (wsum == 0.0) {
        throw std::invalid_argument("weight sum must not be zero");
    }
    return acc / wsum;
}

double median(std::vector<double> values) {
    if (values.empty()) {
        throw std::invalid_argument("values must not be empty");
    }
    const auto mid = values.size() / 2;
    std::nth_element(values.begin(), values.begin() + mid, values.end());
    if (values.size() % 2 != 0) {
        return values[mid];
    }
    const auto left = *std::max_element(values.begin(), values.begin() + mid);
    const auto right = values[mid];
    return (left + right) / 2.0;
}

int clamp_add(int a, int b, int min_value, int max_value) {
    if (min_value > max_value) {
        throw std::invalid_argument("min_value cannot exceed max_value");
    }
    const long long raw = static_cast<long long>(a) + static_cast<long long>(b);
    if (raw < min_value) {
        return min_value;
    }
    if (raw > max_value) {
        return max_value;
    }
    return static_cast<int>(raw);
}

void RunningStats::push(double value) {
    if (count_ == 0) {
        min_ = value;
        max_ = value;
    } else {
        if (value < min_) {
            min_ = value;
        }
        if (value > max_) {
            max_ = value;
        }
    }
    sum_ += value;
    ++count_;
}

void RunningStats::reset() {
    sum_ = 0.0;
    min_ = 0.0;
    max_ = 0.0;
    count_ = 0;
}

std::size_t RunningStats::count() const { return count_; }

double RunningStats::sum() const { return sum_; }

double RunningStats::average() const {
    if (count_ == 0) {
        throw std::invalid_argument("no data to average");
    }
    return sum_ / static_cast<double>(count_);
}

double RunningStats::min() const {
    if (count_ == 0) {
        throw std::invalid_argument("no data for min");
    }
    return min_;
}

double RunningStats::max() const {
    if (count_ == 0) {
        throw std::invalid_argument("no data for max");
    }
    return max_;
}

} // namespace awesome_calc
