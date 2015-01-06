#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include "bptree.h"

int const TEST_MAX_KEY = 100000;
int const NUM_DUPLICATE = 13;

template <class KeyType>
class BPTreeSanityTest
: public ::testing::Test {
public:
    typedef KeyType Key;
    typedef BPTree<std::string, KeyType, 8, 8> Tree;
    typedef BPTree<std::string, KeyType, 60, 61> ManyKeysTree;
};

typedef ::testing::Types<int32_t, int64_t, uint32_t, uint64_t> TreeKeyTypes;
TYPED_TEST_CASE(BPTreeSanityTest, TreeKeyTypes);

TYPED_TEST(BPTreeSanityTest, AscendingInsert) {
    typename TestFixture::Tree tree;

    for (typename TestFixture::Key i = 0; i < TEST_MAX_KEY; i++) {
        tree.insert(i, std::to_string(i));
    }

    for (typename TestFixture::Key i = 0; i < TEST_MAX_KEY; i++) {
        std::string value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(std::to_string(i), value);
    }
}

TYPED_TEST(BPTreeSanityTest, DescendingInsert) {
    typename TestFixture::Tree tree;

    for (typename TestFixture::Key i = TEST_MAX_KEY - 1; i >= 0 && i < TEST_MAX_KEY; i--) {
        tree.insert(i, std::to_string(i));
    }

    for (typename TestFixture::Key i = 0; i < TEST_MAX_KEY; i++) {
        std::string value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(std::to_string(i), value);
    }
}

TYPED_TEST(BPTreeSanityTest, ManyKeysAscendingInsert) {
    typename TestFixture::ManyKeysTree tree;

    for (typename TestFixture::Key i = 0; i < TEST_MAX_KEY; i++) {
        tree.insert(i, std::to_string(i));
    }

    for (typename TestFixture::Key i = 0; i < TEST_MAX_KEY; i++) {
        std::string value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(std::to_string(i), value);
    }
}

TYPED_TEST(BPTreeSanityTest, ManyKeysDescendingInsert) {
    typename TestFixture::ManyKeysTree tree;

    for (typename TestFixture::Key i = TEST_MAX_KEY - 1; i >= 0 && i < TEST_MAX_KEY; i--) {
        tree.insert(i, std::to_string(i));
    }

    for (typename TestFixture::Key i = 0; i < TEST_MAX_KEY; i++) {
        std::string value;
        ASSERT_TRUE(tree.search(i, value));
        EXPECT_EQ(std::to_string(i), value);
    }
}


typedef BPTree<int32_t, int32_t> TestTree;

class BPTreeTest
: public ::testing::Test {
public:
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
