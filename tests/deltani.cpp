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

TEST_F(DeltaFunctionTest, EmptyFunction) {
    DeltaFunction empty;
    DeltaFunction merged1 = d.merge(empty);
    DeltaFunction merged2 = empty.merge(d);

    for (uint32_t i = 1; i <= 8; i++) {
        EXPECT_EQ(i, empty.evaluate(i));
        EXPECT_EQ(i, empty.evaluate_inv(i));

        EXPECT_EQ(d.evaluate(i), merged1.evaluate(i));
        EXPECT_EQ(d.evaluate(i), merged2.evaluate(i));
        EXPECT_EQ(d.evaluate_inv(i), merged1.evaluate_inv(i));
        EXPECT_EQ(d.evaluate_inv(i), merged2.evaluate_inv(i));
    }
}


class DeltaVersionsSanityTest
: public ::testing::Test {
public:
    DeltaVersions versions;

    virtual void SetUp() {
        NIEdgeTree edges;
        edges.insert(1, {1, 1, 8});
        edges.insert(2, {2, 3, 4});
        edges.insert(3, {3, 6, 7});
        edges.insert(4, {4, 2, 5});
        edges.insert(5, {5, 9, 10});
        edges.insert(6, {6, 11, 12});
        versions = DeltaVersions(edges, 9, 12);
    }
};

TEST_F(DeltaVersionsSanityTest, Empty) {
    ASSERT_EQ(0, versions.max_version());

    EXPECT_TRUE(versions.exists(1));
    EXPECT_TRUE(versions.exists(2));
    EXPECT_TRUE(versions.exists(3));
    EXPECT_TRUE(versions.exists(4));
    EXPECT_FALSE(versions.exists(5));
    EXPECT_FALSE(versions.exists(6));

    versions.insert(7, 1);

    EXPECT_TRUE(versions.exists(7));
    EXPECT_TRUE(versions.is_ancestor(1, 7));

    EXPECT_EQ(1, versions.save());
}

TEST_F(DeltaVersionsSanityTest, MultipleInsertOnEmpty) {
    versions.insert(7, 4);
    versions.insert(8, 7);
    versions.insert(9, 4);

    EXPECT_TRUE(versions.exists(1));
    EXPECT_TRUE(versions.exists(2));
    EXPECT_TRUE(versions.exists(3));
    EXPECT_TRUE(versions.exists(4));
    EXPECT_FALSE(versions.exists(5));
    EXPECT_FALSE(versions.exists(6));
    EXPECT_TRUE(versions.exists(7));
    EXPECT_TRUE(versions.exists(8));
    EXPECT_TRUE(versions.exists(9));
}

TEST_F(DeltaVersionsSanityTest, ManyVersions) {
    for (size_t i=1; i <= 10000; i++) {
        DeltaFunction delta;
        delta.add_range({1, 1});
        delta.max = 9;
        versions.insert_delta(delta);
    }
    for (size_t i=1; i <= 10000; i++) {
        EXPECT_TRUE(versions.exists(1, i));
        EXPECT_TRUE(versions.exists(2, i));
        EXPECT_TRUE(versions.exists(3, i));
        EXPECT_TRUE(versions.exists(4, i));
        EXPECT_FALSE(versions.exists(5, i));
        EXPECT_FALSE(versions.exists(6, i));
    }
}

class DeltaVersionsTest
: public DeltaVersionsSanityTest {
public:
    virtual void SetUp() {
        DeltaVersionsSanityTest::SetUp();

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
        v2.add_range({9, 9});
        v2.max = 7;
        versions.insert_delta(v2);

        DeltaFunction v3;
        v3.add_range({1, 1});
        v3.add_range({2, 4});
        v3.add_range({9, 2});
        v3.add_range({7, 9});
        v3.add_range({11, 11});
        v3.max = 9;
        versions.insert_delta(v3);

        DeltaFunction v4;
        v4.add_range({1, 1});
        v4.add_range({2, 3});
        v4.add_range({4, 6});
        v4.add_range({11, 2});
        v4.add_range({12, 5});
        v4.add_range({13, 13});
        v4.max = 11;
        versions.insert_delta(v4);
    }
};

TEST_F(DeltaVersionsTest, MaxVersion) {
    EXPECT_EQ(4, versions.max_version());
}

TEST_F(DeltaVersionsTest, NullVersion) {
    NIEdge e;

    e = versions.get_edge({123, 1, 8}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(8, e.upper);

    e = versions.get_edge({123, 2, 5}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(5, e.upper);

    e = versions.get_edge({123, 3, 4}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(3, e.lower);
    EXPECT_EQ(4, e.upper);

    e = versions.get_edge({123, 6, 7}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(6, e.lower);
    EXPECT_EQ(7, e.upper);

    e = versions.get_edge({123, 9, 10}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(9, e.lower);
    EXPECT_EQ(10, e.upper);

    e = versions.get_edge({123, 11, 12}, 0);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(11, e.lower);
    EXPECT_EQ(12, e.upper);
}

TEST_F(DeltaVersionsTest, SingleStep) {
    NIEdge e;

    // Version 1
    e = versions.get_edge({123, 1, 8}, 1);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(8, e.upper);

    e = versions.get_edge({123, 2, 5}, 1);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(7, e.upper);

    e = versions.get_edge({123, 3, 4}, 1);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(3, e.lower);
    EXPECT_EQ(4, e.upper);

    e = versions.get_edge({123, 6, 7}, 1);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(5, e.lower);
    EXPECT_EQ(6, e.upper);

    e = versions.get_edge({123, 9, 10}, 1);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(9, e.lower);
    EXPECT_EQ(10, e.upper);

    e = versions.get_edge({123, 11, 12}, 1);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(11, e.lower);
    EXPECT_EQ(12, e.upper);

    // Version 2
    e = versions.get_edge({123, 1, 8}, 2);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(6, e.upper);

    e = versions.get_edge({123, 2, 5}, 2);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(5, e.upper);

    e = versions.get_edge({123, 3, 4}, 2);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(7, e.lower);
    EXPECT_EQ(8, e.upper);

    e = versions.get_edge({123, 6, 7}, 2);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(3, e.lower);
    EXPECT_EQ(4, e.upper);

    e = versions.get_edge({123, 9, 10}, 2);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(9, e.lower);
    EXPECT_EQ(10, e.upper);

    e = versions.get_edge({123, 11, 12}, 2);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(11, e.lower);
    EXPECT_EQ(12, e.upper);

    // Version 4
    e = versions.get_edge({123, 1, 8}, 4);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(10, e.upper);

    e = versions.get_edge({123, 2, 5}, 4);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(6, e.lower);
    EXPECT_EQ(9, e.upper);

    e = versions.get_edge({123, 3, 4}, 4);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(11, e.lower);
    EXPECT_EQ(12, e.upper);

    e = versions.get_edge({123, 6, 7}, 4);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(7, e.lower);
    EXPECT_EQ(8, e.upper);

    e = versions.get_edge({123, 9, 10}, 4);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(3, e.lower);
    EXPECT_EQ(4, e.upper);

    e = versions.get_edge({123, 11, 12}, 4);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(5, e.upper);
}

TEST_F(DeltaVersionsTest, MultipleStep) {
    NIEdge e;

    e = versions.get_edge({123, 1, 8}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(1, e.lower);
    EXPECT_EQ(8, e.upper);

    e = versions.get_edge({123, 2, 5}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(4, e.lower);
    EXPECT_EQ(7, e.upper);

    e = versions.get_edge({123, 3, 4}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(9, e.lower);
    EXPECT_EQ(10, e.upper);

    e = versions.get_edge({123, 6, 7}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(5, e.lower);
    EXPECT_EQ(6, e.upper);

    e = versions.get_edge({123, 9, 10}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(2, e.lower);
    EXPECT_EQ(3, e.upper);

    e = versions.get_edge({123, 11, 12}, 3);
    EXPECT_EQ(123, e.loc_id);
    EXPECT_EQ(11, e.lower);
    EXPECT_EQ(12, e.upper);
}

TEST_F(DeltaVersionsTest, Exists) {
    // Version 0
    EXPECT_TRUE(versions.exists(1, 0));
    EXPECT_TRUE(versions.exists(2, 0));
    EXPECT_TRUE(versions.exists(3, 0));
    EXPECT_TRUE(versions.exists(4, 0));
    EXPECT_FALSE(versions.exists(5, 0));
    EXPECT_FALSE(versions.exists(6, 0));

    // Version 1
    EXPECT_TRUE(versions.exists(1, 1));
    EXPECT_TRUE(versions.exists(2, 1));
    EXPECT_TRUE(versions.exists(3, 1));
    EXPECT_TRUE(versions.exists(4, 1));
    EXPECT_FALSE(versions.exists(5, 1));
    EXPECT_FALSE(versions.exists(6, 1));

    // Version 2
    EXPECT_TRUE(versions.exists(1, 2));
    EXPECT_FALSE(versions.exists(2, 2));
    EXPECT_TRUE(versions.exists(3, 2));
    EXPECT_TRUE(versions.exists(4, 2));
    EXPECT_FALSE(versions.exists(5, 2));
    EXPECT_FALSE(versions.exists(6, 2));

    // Version 3
    EXPECT_TRUE(versions.exists(1, 3));
    EXPECT_FALSE(versions.exists(2, 3));
    EXPECT_TRUE(versions.exists(3, 3));
    EXPECT_TRUE(versions.exists(4, 3));
    EXPECT_TRUE(versions.exists(5, 3));
    EXPECT_FALSE(versions.exists(6, 3));

    // Version 4
    EXPECT_TRUE(versions.exists(1, 4));
    EXPECT_FALSE(versions.exists(2, 4));
    EXPECT_TRUE(versions.exists(3, 4));
    EXPECT_TRUE(versions.exists(4, 4));
    EXPECT_TRUE(versions.exists(5, 4));
    EXPECT_TRUE(versions.exists(6, 4));

    // Implicit Version 4
    EXPECT_TRUE(versions.exists(1));
    EXPECT_FALSE(versions.exists(2));
    EXPECT_TRUE(versions.exists(3));
    EXPECT_TRUE(versions.exists(4));
    EXPECT_TRUE(versions.exists(5));
    EXPECT_TRUE(versions.exists(6));
}

TEST_F(DeltaVersionsTest, IsAncestor) {
    EXPECT_TRUE(versions.is_ancestor(1, 4, 0));
    EXPECT_TRUE(versions.is_ancestor(1, 3, 0));
    EXPECT_TRUE(versions.is_ancestor(1, 2, 0));
    EXPECT_TRUE(versions.is_ancestor(4, 2, 0));

    EXPECT_TRUE(versions.is_ancestor(1, 4, 1));
    EXPECT_TRUE(versions.is_ancestor(1, 2, 1));
    EXPECT_TRUE(versions.is_ancestor(1, 3, 1));
    EXPECT_TRUE(versions.is_ancestor(4, 2, 1));
    EXPECT_TRUE(versions.is_ancestor(4, 3, 1));

    EXPECT_TRUE(versions.is_ancestor(1, 4, 2));
    EXPECT_TRUE(versions.is_ancestor(1, 3, 2));
    EXPECT_TRUE(versions.is_ancestor(4, 3, 2));

    EXPECT_TRUE(versions.is_ancestor(1, 5, 3));
    EXPECT_TRUE(versions.is_ancestor(1, 4, 3));
    EXPECT_TRUE(versions.is_ancestor(1, 3, 3));
    EXPECT_TRUE(versions.is_ancestor(4, 3, 3));

    EXPECT_TRUE(versions.is_ancestor(1, 6, 4));
    EXPECT_TRUE(versions.is_ancestor(1, 4, 4));
    EXPECT_TRUE(versions.is_ancestor(1, 5, 4));
    EXPECT_TRUE(versions.is_ancestor(1, 3, 4));
    EXPECT_TRUE(versions.is_ancestor(6, 5, 4));
    EXPECT_TRUE(versions.is_ancestor(4, 3, 4));

    // implicit max version
    EXPECT_TRUE(versions.is_ancestor(1, 6));
    EXPECT_TRUE(versions.is_ancestor(1, 4));
    EXPECT_TRUE(versions.is_ancestor(1, 5));
    EXPECT_TRUE(versions.is_ancestor(1, 3));
    EXPECT_TRUE(versions.is_ancestor(6, 5));
    EXPECT_TRUE(versions.is_ancestor(4, 3));
}

TEST_F(DeltaVersionsTest, NotIsAncestor) {
    EXPECT_FALSE(versions.is_ancestor(1, 1, 0));
    EXPECT_FALSE(versions.is_ancestor(1, 5, 0));
    EXPECT_FALSE(versions.is_ancestor(1, 6, 0));
    EXPECT_FALSE(versions.is_ancestor(2, 1, 0));
    EXPECT_FALSE(versions.is_ancestor(2, 2, 0));
    EXPECT_FALSE(versions.is_ancestor(2, 3, 0));
    EXPECT_FALSE(versions.is_ancestor(2, 4, 0));
    EXPECT_FALSE(versions.is_ancestor(2, 5, 0));
    EXPECT_FALSE(versions.is_ancestor(2, 6, 0));
    EXPECT_FALSE(versions.is_ancestor(3, 1, 0));
    EXPECT_FALSE(versions.is_ancestor(3, 2, 0));
    EXPECT_FALSE(versions.is_ancestor(3, 3, 0));
    EXPECT_FALSE(versions.is_ancestor(3, 4, 0));
    EXPECT_FALSE(versions.is_ancestor(3, 5, 0));
    EXPECT_FALSE(versions.is_ancestor(3, 6, 0));
    EXPECT_FALSE(versions.is_ancestor(4, 1, 0));
    EXPECT_FALSE(versions.is_ancestor(4, 3, 0));
    EXPECT_FALSE(versions.is_ancestor(4, 4, 0));
    EXPECT_FALSE(versions.is_ancestor(4, 5, 0));
    EXPECT_FALSE(versions.is_ancestor(4, 6, 0));
    EXPECT_FALSE(versions.is_ancestor(5, 1, 0));
    EXPECT_FALSE(versions.is_ancestor(5, 2, 0));
    EXPECT_FALSE(versions.is_ancestor(5, 3, 0));
    EXPECT_FALSE(versions.is_ancestor(5, 4, 0));
    EXPECT_FALSE(versions.is_ancestor(5, 5, 0));
    EXPECT_FALSE(versions.is_ancestor(5, 6, 0));
    EXPECT_FALSE(versions.is_ancestor(6, 1, 0));
    EXPECT_FALSE(versions.is_ancestor(6, 2, 0));
    EXPECT_FALSE(versions.is_ancestor(6, 3, 0));
    EXPECT_FALSE(versions.is_ancestor(6, 4, 0));
    EXPECT_FALSE(versions.is_ancestor(6, 5, 0));
    EXPECT_FALSE(versions.is_ancestor(6, 6, 0));

    EXPECT_FALSE(versions.is_ancestor(1, 1, 1));
    EXPECT_FALSE(versions.is_ancestor(1, 5, 1));
    EXPECT_FALSE(versions.is_ancestor(1, 6, 1));
    EXPECT_FALSE(versions.is_ancestor(2, 1, 1));
    EXPECT_FALSE(versions.is_ancestor(2, 2, 1));
    EXPECT_FALSE(versions.is_ancestor(2, 3, 1));
    EXPECT_FALSE(versions.is_ancestor(2, 4, 1));
    EXPECT_FALSE(versions.is_ancestor(2, 5, 1));
    EXPECT_FALSE(versions.is_ancestor(2, 6, 1));
    EXPECT_FALSE(versions.is_ancestor(3, 1, 1));
    EXPECT_FALSE(versions.is_ancestor(3, 2, 1));
    EXPECT_FALSE(versions.is_ancestor(3, 3, 1));
    EXPECT_FALSE(versions.is_ancestor(3, 4, 1));
    EXPECT_FALSE(versions.is_ancestor(3, 5, 1));
    EXPECT_FALSE(versions.is_ancestor(3, 6, 1));
    EXPECT_FALSE(versions.is_ancestor(4, 1, 1));
    EXPECT_FALSE(versions.is_ancestor(4, 4, 1));
    EXPECT_FALSE(versions.is_ancestor(4, 5, 1));
    EXPECT_FALSE(versions.is_ancestor(4, 6, 1));
    EXPECT_FALSE(versions.is_ancestor(5, 1, 1));
    EXPECT_FALSE(versions.is_ancestor(5, 2, 1));
    EXPECT_FALSE(versions.is_ancestor(5, 3, 1));
    EXPECT_FALSE(versions.is_ancestor(5, 4, 1));
    EXPECT_FALSE(versions.is_ancestor(5, 5, 1));
    EXPECT_FALSE(versions.is_ancestor(5, 6, 1));
    EXPECT_FALSE(versions.is_ancestor(6, 1, 1));
    EXPECT_FALSE(versions.is_ancestor(6, 2, 1));
    EXPECT_FALSE(versions.is_ancestor(6, 3, 1));
    EXPECT_FALSE(versions.is_ancestor(6, 4, 1));
    EXPECT_FALSE(versions.is_ancestor(6, 5, 1));
    EXPECT_FALSE(versions.is_ancestor(6, 6, 1));

    EXPECT_FALSE(versions.is_ancestor(1, 1, 2));
    EXPECT_FALSE(versions.is_ancestor(1, 2, 2));
    EXPECT_FALSE(versions.is_ancestor(1, 5, 2));
    EXPECT_FALSE(versions.is_ancestor(1, 6, 2));
    EXPECT_FALSE(versions.is_ancestor(2, 1, 2));
    EXPECT_FALSE(versions.is_ancestor(2, 2, 2));
    EXPECT_FALSE(versions.is_ancestor(2, 3, 2));
    EXPECT_FALSE(versions.is_ancestor(2, 4, 2));
    EXPECT_FALSE(versions.is_ancestor(2, 5, 2));
    EXPECT_FALSE(versions.is_ancestor(2, 6, 2));
    EXPECT_FALSE(versions.is_ancestor(3, 1, 2));
    EXPECT_FALSE(versions.is_ancestor(3, 2, 2));
    EXPECT_FALSE(versions.is_ancestor(3, 3, 2));
    EXPECT_FALSE(versions.is_ancestor(3, 4, 2));
    EXPECT_FALSE(versions.is_ancestor(3, 5, 2));
    EXPECT_FALSE(versions.is_ancestor(3, 6, 2));
    EXPECT_FALSE(versions.is_ancestor(4, 1, 2));
    EXPECT_FALSE(versions.is_ancestor(4, 2, 2));
    EXPECT_FALSE(versions.is_ancestor(4, 4, 2));
    EXPECT_FALSE(versions.is_ancestor(4, 5, 2));
    EXPECT_FALSE(versions.is_ancestor(4, 6, 2));
    EXPECT_FALSE(versions.is_ancestor(5, 1, 2));
    EXPECT_FALSE(versions.is_ancestor(5, 2, 2));
    EXPECT_FALSE(versions.is_ancestor(5, 3, 2));
    EXPECT_FALSE(versions.is_ancestor(5, 4, 2));
    EXPECT_FALSE(versions.is_ancestor(5, 5, 2));
    EXPECT_FALSE(versions.is_ancestor(5, 6, 2));
    EXPECT_FALSE(versions.is_ancestor(6, 1, 2));
    EXPECT_FALSE(versions.is_ancestor(6, 2, 2));
    EXPECT_FALSE(versions.is_ancestor(6, 3, 2));
    EXPECT_FALSE(versions.is_ancestor(6, 4, 2));
    EXPECT_FALSE(versions.is_ancestor(6, 5, 2));
    EXPECT_FALSE(versions.is_ancestor(6, 6, 2));

    EXPECT_FALSE(versions.is_ancestor(1, 1, 3));
    EXPECT_FALSE(versions.is_ancestor(1, 2, 3));
    EXPECT_FALSE(versions.is_ancestor(1, 6, 3));
    EXPECT_FALSE(versions.is_ancestor(2, 1, 3));
    EXPECT_FALSE(versions.is_ancestor(2, 2, 3));
    EXPECT_FALSE(versions.is_ancestor(2, 3, 3));
    EXPECT_FALSE(versions.is_ancestor(2, 4, 3));
    EXPECT_FALSE(versions.is_ancestor(2, 5, 3));
    EXPECT_FALSE(versions.is_ancestor(2, 6, 3));
    EXPECT_FALSE(versions.is_ancestor(3, 1, 3));
    EXPECT_FALSE(versions.is_ancestor(3, 2, 3));
    EXPECT_FALSE(versions.is_ancestor(3, 3, 3));
    EXPECT_FALSE(versions.is_ancestor(3, 4, 3));
    EXPECT_FALSE(versions.is_ancestor(3, 5, 3));
    EXPECT_FALSE(versions.is_ancestor(3, 6, 3));
    EXPECT_FALSE(versions.is_ancestor(4, 1, 3));
    EXPECT_FALSE(versions.is_ancestor(4, 2, 3));
    EXPECT_FALSE(versions.is_ancestor(4, 4, 3));
    EXPECT_FALSE(versions.is_ancestor(4, 5, 3));
    EXPECT_FALSE(versions.is_ancestor(4, 6, 3));
    EXPECT_FALSE(versions.is_ancestor(5, 1, 3));
    EXPECT_FALSE(versions.is_ancestor(5, 2, 3));
    EXPECT_FALSE(versions.is_ancestor(5, 3, 3));
    EXPECT_FALSE(versions.is_ancestor(5, 4, 3));
    EXPECT_FALSE(versions.is_ancestor(5, 5, 3));
    EXPECT_FALSE(versions.is_ancestor(5, 6, 3));
    EXPECT_FALSE(versions.is_ancestor(6, 1, 3));
    EXPECT_FALSE(versions.is_ancestor(6, 2, 3));
    EXPECT_FALSE(versions.is_ancestor(6, 3, 3));
    EXPECT_FALSE(versions.is_ancestor(6, 4, 3));
    EXPECT_FALSE(versions.is_ancestor(6, 5, 3));
    EXPECT_FALSE(versions.is_ancestor(6, 6, 3));

    EXPECT_FALSE(versions.is_ancestor(1, 1, 4));
    EXPECT_FALSE(versions.is_ancestor(1, 2, 4));
    EXPECT_FALSE(versions.is_ancestor(2, 1, 4));
    EXPECT_FALSE(versions.is_ancestor(2, 2, 4));
    EXPECT_FALSE(versions.is_ancestor(2, 3, 4));
    EXPECT_FALSE(versions.is_ancestor(2, 4, 4));
    EXPECT_FALSE(versions.is_ancestor(2, 5, 4));
    EXPECT_FALSE(versions.is_ancestor(2, 6, 4));
    EXPECT_FALSE(versions.is_ancestor(3, 1, 4));
    EXPECT_FALSE(versions.is_ancestor(3, 2, 4));
    EXPECT_FALSE(versions.is_ancestor(3, 3, 4));
    EXPECT_FALSE(versions.is_ancestor(3, 4, 4));
    EXPECT_FALSE(versions.is_ancestor(3, 5, 4));
    EXPECT_FALSE(versions.is_ancestor(3, 6, 4));
    EXPECT_FALSE(versions.is_ancestor(4, 1, 4));
    EXPECT_FALSE(versions.is_ancestor(4, 2, 4));
    EXPECT_FALSE(versions.is_ancestor(4, 4, 4));
    EXPECT_FALSE(versions.is_ancestor(4, 5, 4));
    EXPECT_FALSE(versions.is_ancestor(4, 6, 4));
    EXPECT_FALSE(versions.is_ancestor(5, 1, 4));
    EXPECT_FALSE(versions.is_ancestor(5, 2, 4));
    EXPECT_FALSE(versions.is_ancestor(5, 3, 4));
    EXPECT_FALSE(versions.is_ancestor(5, 4, 4));
    EXPECT_FALSE(versions.is_ancestor(5, 5, 4));
    EXPECT_FALSE(versions.is_ancestor(5, 6, 4));
    EXPECT_FALSE(versions.is_ancestor(6, 1, 4));
    EXPECT_FALSE(versions.is_ancestor(6, 2, 4));
    EXPECT_FALSE(versions.is_ancestor(6, 3, 4));
    EXPECT_FALSE(versions.is_ancestor(6, 4, 4));
    EXPECT_FALSE(versions.is_ancestor(6, 6, 4));
}

TEST_F(DeltaVersionsTest, Insert) {
    EXPECT_THROW(versions.insert(7, 100), deltani_invalid_id);
    EXPECT_THROW(versions.insert(3, 1), deltani_id_exists);

    versions.insert(7, 3);

    EXPECT_TRUE(versions.exists(7));
    EXPECT_TRUE(versions.is_ancestor(3, 7));
    EXPECT_TRUE(versions.is_ancestor(1, 7));
}

TEST_F(DeltaVersionsTest, Remove) {
    EXPECT_THROW(versions.remove(100), deltani_invalid_id);
    EXPECT_THROW(versions.remove(2), deltani_id_removed);
    EXPECT_THROW(versions.remove(4), deltani_id_has_children);

    versions.remove(5);

    EXPECT_FALSE(versions.exists(5));
    EXPECT_FALSE(versions.is_ancestor(6, 5));
    EXPECT_FALSE(versions.is_ancestor(1, 5));
    EXPECT_TRUE(versions.is_ancestor(1, 6));
}

TEST_F(DeltaVersionsTest, RemoveAll) {
    versions.remove(3);
    versions.remove(4);
    versions.remove(5);
    versions.remove(6);
    versions.remove(1);

    EXPECT_FALSE(versions.exists(1));
    EXPECT_FALSE(versions.exists(2));
    EXPECT_FALSE(versions.exists(3));
    EXPECT_FALSE(versions.exists(4));
    EXPECT_FALSE(versions.exists(5));
    EXPECT_FALSE(versions.exists(6));
}

TEST_F(DeltaVersionsTest, Save) {
    EXPECT_EQ(4, versions.save());

    versions.insert(7, 3);

    EXPECT_EQ(5, versions.save());
    EXPECT_TRUE(versions.exists(7, 5));
    EXPECT_TRUE(versions.is_ancestor(3, 7, 5));
    EXPECT_TRUE(versions.is_ancestor(1, 7, 5));
}
