#include <gtest/gtest.h>
#include <tuple>
#include <vector>

#include "awesome_calc/calc.hpp"

// ---- Простые тесты без фикстур ----

TEST(MeanTest, NonEmpty) {
    std::vector<double> v{1.0, 2.0, 3.0};
    EXPECT_DOUBLE_EQ(awesome_calc::mean(v), 2.0);
}

TEST(MeanTest, ThrowsOnEmpty) {
    std::vector<double> v;
    EXPECT_THROW(awesome_calc::mean(v), std::invalid_argument);
}

TEST(WeightedMeanTest, HappyPath) {
    std::vector<double> v{1.0, 2.0, 3.0};
    std::vector<double> w{0.2, 0.3, 0.5};
    EXPECT_DOUBLE_EQ(awesome_calc::weighted_mean(v, w), 2.3);
}

TEST(WeightedMeanTest, ThrowsOnMismatch) {
    std::vector<double> v{1.0, 2.0};
    std::vector<double> w{0.5};
    EXPECT_THROW(awesome_calc::weighted_mean(v, w), std::invalid_argument);
}

TEST(WeightedMeanTest, ThrowsOnZeroWeightSum) {
    std::vector<double> v{1.0, 2.0};
    std::vector<double> w{1.0, -1.0};
    EXPECT_THROW(awesome_calc::weighted_mean(v, w), std::invalid_argument);
}

TEST(MedianTest, OddCount) {
    std::vector<double> v{5.0, 1.0, 3.0};
    EXPECT_DOUBLE_EQ(awesome_calc::median(v), 3.0);
}

TEST(MedianTest, EvenCount) {
    std::vector<double> v{5.0, 1.0, 3.0, 7.0};
    EXPECT_DOUBLE_EQ(awesome_calc::median(v), 4.0);
}

// ---- Параметризованные тесты для clamp_add ----
using ClampParams = std::tuple<int, int, int, int, int>;

class ClampAddTest : public ::testing::TestWithParam<ClampParams> {};

TEST_P(ClampAddTest, SaturatesWithinBounds) {
    const auto [a, b, mn, mx, expected] = GetParam();
    EXPECT_EQ(awesome_calc::clamp_add(a, b, mn, mx), expected);
}

INSTANTIATE_TEST_SUITE_P(
    ClampCases,
    ClampAddTest,
    ::testing::Values(
        ClampParams{1, 2, -10, 10, 3},           // внутри диапазона
        ClampParams{100, 50, -10, 120, 120},     // верхняя насыщенность
        ClampParams{-100, -50, -120, 10, -120},  // нижняя насыщенность
        ClampParams{0, 0, -1, 1, 0}));           // на границе

TEST(ClampAddTest, ThrowsOnInvalidBounds) {
    EXPECT_THROW(awesome_calc::clamp_add(1, 2, 10, -10), std::invalid_argument);
}

// ---- Stateful API: фикстура для RunningStats ----
class RunningStatsTest : public ::testing::Test {
  protected:
    void SetUp() override {
        stats.push(10.0);
        stats.push(20.0);
        stats.push(30.0);
    }

    awesome_calc::RunningStats stats;
};

TEST_F(RunningStatsTest, AggregatesBasicStats) {
    EXPECT_EQ(stats.count(), 3u);
    EXPECT_DOUBLE_EQ(stats.sum(), 60.0);
    EXPECT_DOUBLE_EQ(stats.average(), 20.0);
    EXPECT_DOUBLE_EQ(stats.min(), 10.0);
    EXPECT_DOUBLE_EQ(stats.max(), 30.0);
}

TEST_F(RunningStatsTest, ResetClearsState) {
    stats.reset();
    EXPECT_EQ(stats.count(), 0u);
    EXPECT_DOUBLE_EQ(stats.sum(), 0.0);
    EXPECT_THROW(stats.average(), std::invalid_argument);
}

// Покрываем базовую арифметику и выброс исключений.
TEST(AddTest, SimpleValues) {
    EXPECT_EQ(awesome_calc::add(1, 2), 3);
    EXPECT_EQ(awesome_calc::add(-1, 1), 0);
    EXPECT_EQ(awesome_calc::add(-5, -7), -12);
}

// Определяем точку входа сами, чтобы не зависеть от наличия gtest_main в среде.
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
