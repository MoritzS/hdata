#include <cstdint>
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

TEST_F(DeltaFunctionTest, Merge) {
    DeltaFunction m;
    DeltaFunction merged;

    m.add_range({1, 1});
    m.add_range({3, 7});
    m.add_range({5, 3});
    m.max = 7;

    merged = d.merge(m);

    EXPECT_EQ(7, merged.max);

    for (uint32_t i = 1; i <= 8; i++) {
        EXPECT_EQ(m.evaluate(d.evaluate(i)), merged.evaluate(i));
    }
}


class DeltaVersionsTest
: public ::testing::Test {
public:
    DeltaVersions versions;

    virtual void SetUp() {
        DeltaFunction v1;
        v1.add_range({1, 1});
        v1.add_range({5, 7});
        v1.add_range({6, 5});
        v1.add_range({8, 8});
        v1.max = 9;
        versions.insert_delta(v1);

        DeltaFunction v2;
        v2.add_range({1, 1});
        v2.add_range({3, 7});
        v2.add_range({5, 3});
        v2.max = 7;
        versions.insert_delta(v2);

        DeltaFunction v3;
        v3.add_range({1, 1});
        v3.add_range({2, 4});
        v3.add_range({9, 2});
        v3.max = 9;
        versions.insert_delta(v3);

        DeltaFunction v4;
        v4.add_range({1, 1});
        v4.add_range({2, 3});
        v4.add_range({4, 6});
        v4.add_range({11, 2});
        v4.add_range({12, 5});
        v4.max = 11;
        versions.insert_delta(v4);
    }
};

TEST_F(DeltaVersionsTest, MaxVersion) {
    EXPECT_EQ(4, versions.max_version());
}

TEST_F(DeltaVersionsTest, NullVersion) {
    NIEdge e;

    e = versions.get_version({123, 1, 8}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(8, e.upper);

    e = versions.get_version({123, 2, 5}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(5, e.upper);

    e = versions.get_version({123, 3, 4}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(3, e.lower);
    EXPECT_EQ(4, e.upper);

    e = versions.get_version({123, 6, 7}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(6, e.lower);
    EXPECT_EQ(7, e.upper);
}

TEST_F(DeltaVersionsTest, SingleStep) {
    NIEdge e;

    e = versions.get_version({123, 6, 7}, 1);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(5, e.lower);
    EXPECT_EQ(6, e.upper);

    e = versions.get_version({123, 3, 4}, 2);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(7, e.lower);
    EXPECT_EQ(8, e.upper);

    e = versions.get_version({123, 2, 5}, 4);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(6, e.lower);
    EXPECT_EQ(9, e.upper);
}

TEST_F(DeltaVersionsTest, MultipleStep) {
    NIEdge e;

    e = versions.get_version({123, 2, 5}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(4, e.lower);
    EXPECT_EQ(7, e.upper);

    e = versions.get_version({123, 3, 4}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(9, e.lower);
    EXPECT_EQ(10, e.upper);

    e = versions.get_version({123, 6, 7}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(5, e.lower);
    EXPECT_EQ(6, e.upper);
}
