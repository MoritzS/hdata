#include <gtest/gtest.h>
#include "locations.h"

class DeltaniTest
: public ::testing::Test {
public:
    DeltaFunction d;

    virtual void SetUp() {
        d.ranges.insert(1, {1, 1});
        d.ranges.insert(3, {3, 5});
        d.ranges.insert(6, {6, 3});
        d.ranges.insert(8, {8, 8});
        d.max = 9;
    }
};

TEST_F(DeltaniTest, SimpleApply) {
    NIEdge e;

    e = d.apply({123, 1, 2});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(2, e.upper);

    e = d.apply({123, 1, 8});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(8, e.upper);

    e = d.apply({123, 2, 8});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(8, e.upper);
}

TEST_F(DeltaniTest, AdvancedApply) {
    NIEdge e;

    e = d.apply({123, 1, 4});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(6, e.upper);

    e = d.apply({123, 1, 6});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(3, e.upper);

    e = d.apply({123, 2, 3});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(5, e.upper);

    e = d.apply({123, 2, 5});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(7, e.upper);

    e = d.apply({123, 2, 7});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(4, e.upper);

    e = d.apply({123, 3, 4});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(5, e.lower);
    EXPECT_EQ(6, e.upper);

    e = d.apply({123, 4, 5});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(6, e.lower);
    EXPECT_EQ(7, e.upper);

    e = d.apply({123, 6, 7});
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(3, e.lower);
    EXPECT_EQ(4, e.upper);
}
