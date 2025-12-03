#include "awesome_calc/calc.hpp"

#include <gtest/gtest.h>
#include <vector>

using namespace awesome_calc;

TEST(AddTest, SimpleValues)
{
    EXPECT_EQ(add(1, 2), 3);
    EXPECT_EQ(add(-1, 1), 0);
    EXPECT_EQ(add(-5, -7), -12);
}

TEST(MeanTest, NonEmpty)
{
    std::vector<double> v{1.0, 2.0, 3.0};
    EXPECT_DOUBLE_EQ(mean(v), 2.0);
}

TEST(MeanTest, ThrowsOnEmpty)
{
    std::vector<double> v;
    EXPECT_THROW(mean(v), std::invalid_argument);
}
