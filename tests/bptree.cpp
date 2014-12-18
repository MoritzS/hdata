#include <cstdint>
#include <gtest/gtest.h>
#include "bptree.h"

int const TEST_MAX_KEY = 100000;
int const NUM_DUPLICATE = 13;

typedef BPTree<int32_t, int32_t, 8, 8> TestTree;
typedef BPTree<int32_t, int32_t, 60, 61> ManyKeysTree;

TEST(BPTreeSanityTest, AscendingInsert) {
    TestTree tree;

    for (int32_t i = 0; i < TEST_MAX_KEY; i++) {
        tree.insert(i, i);
    }

    for (int32_t i = 0; i < TEST_MAX_KEY; i++) {
        int32_t value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(i, value);
    }
}

TEST(BPTreeSanityTest, DescendingInsert) {
    TestTree tree;

    for (int32_t i = TEST_MAX_KEY - 1; i >=0; i--) {
        tree.insert(i, i);
    }

    for (int32_t i = 0; i < TEST_MAX_KEY; i++) {
        int32_t value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(i, value);
    }
}

TEST(BPTreeSanityTest, ManyKeysAscendingInsert) {
    ManyKeysTree tree;

    for (int32_t i = 0; i < TEST_MAX_KEY; i++) {
        tree.insert(i, i);
    }

    for (int32_t i = 0; i < TEST_MAX_KEY; i++) {
        int32_t value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(i, value);
    }
}

TEST(BPTreeSanityTest, ManyKeysDescendingInsert) {
    ManyKeysTree tree;

    for (int32_t i = TEST_MAX_KEY - 1; i >=0; i--) {
        tree.insert(i, i);
    }

    for (int32_t i = 0; i < TEST_MAX_KEY; i++) {
        int32_t value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(i, value);
    }
}

class BPTreeTest
: public ::testing::Test {
protected:
    TestTree tree;

    virtual void SetUp() {
        for (int32_t i = 5; i <= TEST_MAX_KEY; i += 5) {
            for (int32_t j = 1; j <= 5; j++) {
                int32_t key = i - j;
                for (int k=0; k < NUM_DUPLICATE; k++) {
                    tree.insert(key, key);
                }
            }
        }
    }
};

TEST_F(BPTreeTest, PseudoRandomInsert) {
    for (int32_t i=0; i < TEST_MAX_KEY; i++) {
        int32_t value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(i, value);
    }
}

TEST_F(BPTreeTest, CountKeyTest) {
    for (int32_t i=0; i < TEST_MAX_KEY; i++) {
        size_t count = tree.count_key(i);
        EXPECT_EQ(NUM_DUPLICATE, count);
    }
}

TEST_F(BPTreeTest, EmptySearchTest) {
    EXPECT_FALSE(tree.search_iter(TEST_MAX_KEY / 2).empty());
    EXPECT_TRUE(tree.search_iter(TEST_MAX_KEY + 100).empty());
    EXPECT_TRUE(tree.search_iter(-100).empty());

    TestTree empty_tree;
    EXPECT_TRUE(empty_tree.search_range(0).empty());
}

TEST_F(BPTreeTest, OutOfRangeTest) {
    EXPECT_EQ(0, *tree.search_range(-100).begin());
    EXPECT_EQ(TEST_MAX_KEY - 1, *tree.search_range(TEST_MAX_KEY+100).begin());
}
