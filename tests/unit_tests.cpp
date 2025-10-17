#include <gtest/gtest.h>
#include "math_operations.h"

// Basic tests for add(int, int)


// Simple sample test - replace/add real tests here
TEST(SampleTest, Addition) {
  EXPECT_EQ(2 + 2, 4);
}

TEST(AddTest, PositiveNumbers) {
    EXPECT_EQ(add(1, 2), 3);
    EXPECT_EQ(add(100, 200), 300);
}

TEST(AddTest, NegativeNumbers) {
    EXPECT_EQ(add(-1, -2), -3);
    EXPECT_EQ(add(-100, -200), -300);
}

TEST(AddTest, MixedSign) {
    EXPECT_EQ(add(-5, 5), 0);
    EXPECT_EQ(add(10, -3), 7);
}

TEST(AddTest, WithZero) {
    EXPECT_EQ(add(0, 0), 0);
    EXPECT_EQ(add(0, 5), 5);
    EXPECT_EQ(add(7, 0), 7);
}

TEST(AddTest, LargeValues) {
    // These values are chosen to avoid undefined behaviour from signed overflow.
    EXPECT_EQ(add(1000000, 2000000), 3000000);
    EXPECT_EQ(add(-1000000, -2000000), -3000000);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
