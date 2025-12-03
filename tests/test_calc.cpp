#include <gtest/gtest.h>
#include <vector>

#include "awesome_calc/calc.hpp"

// Покрываем базовую арифметику и выброс исключений.
TEST(AddTest, SimpleValues) {
    EXPECT_EQ(awesome_calc::add(1, 2), 3);
    EXPECT_EQ(awesome_calc::add(-1, 1), 0);
    EXPECT_EQ(awesome_calc::add(-5, -7), -12);
}

TEST(MeanTest, NonEmpty) {
    std::vector<double> v{1.0, 2.0, 3.0};
    EXPECT_DOUBLE_EQ(awesome_calc::mean(v), 2.0);
}

TEST(MeanTest, ThrowsOnEmpty) {
    std::vector<double> v;
    EXPECT_THROW(awesome_calc::mean(v), std::invalid_argument);
}

// Определяем точку входа сами, чтобы не зависеть от наличия gtest_main в среде.
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
