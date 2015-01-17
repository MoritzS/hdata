#include <gtest/gtest.h>
#include "locations.h"

class DeltaFunctionTest
: public ::testing::Test {
public:
    DeltaFunction d;

    virtual void SetUp() {
        d.add_range({1, 1});
        d.add_range({5, 7});
        d.add_range({6, 5});
        d.add_range({8, 8});
        d.max = 9;
    }
};

TEST_F(DeltaFunctionTest, Evaluate) {
    EXPECT_EQ(1, d.evaluate(1));
    EXPECT_EQ(2, d.evaluate(2));
    EXPECT_EQ(3, d.evaluate(3));
    EXPECT_EQ(4, d.evaluate(4));
    EXPECT_EQ(7, d.evaluate(5));
    EXPECT_EQ(5, d.evaluate(6));
    EXPECT_EQ(6, d.evaluate(7));
    EXPECT_EQ(8, d.evaluate(8));
    EXPECT_EQ(9, d.evaluate(9));
}

TEST_F(DeltaFunctionTest, EvaluateInv) {
    EXPECT_EQ(1, d.evaluate_inv(1));
    EXPECT_EQ(2, d.evaluate_inv(2));
    EXPECT_EQ(3, d.evaluate_inv(3));
    EXPECT_EQ(4, d.evaluate_inv(4));
    EXPECT_EQ(5, d.evaluate_inv(7));
    EXPECT_EQ(6, d.evaluate_inv(5));
    EXPECT_EQ(7, d.evaluate_inv(6));
    EXPECT_EQ(8, d.evaluate_inv(8));
    EXPECT_EQ(9, d.evaluate_inv(9));
}

TEST_F(DeltaFunctionTest, SimpleApply) {
    NIEdge e;

    e = d.apply({123, 1, 8});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(8, e.upper);

    e = d.apply({123, 3, 4});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(3, e.lower);
    EXPECT_EQ(4, e.upper);
}

TEST_F(DeltaFunctionTest, AdvancedApply) {
    NIEdge e;

    e = d.apply({123, 2, 5});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(7, e.upper);

    e = d.apply({123, 6, 7});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(5, e.lower);
    EXPECT_EQ(6, e.upper);
}
