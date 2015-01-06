#include <cstdint>
#include <limits>
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


template <class T>
class BPTreeTest
: public ::testing::Test {
public:
    typedef T KeyType;
    typedef BPTree<T, T> TestTree;
    TestTree tree;

    virtual void SetUp() {
        for (KeyType i = 6; i <= TEST_MAX_KEY + 1; i += 5) {
            for (KeyType j = 1; j <= 5; j++) {
                KeyType key = i - j;
                for (int k=0; k < NUM_DUPLICATE; k++) {
                    tree.insert(key, key);
                }
            }
        }
    }
};

TYPED_TEST_CASE(BPTreeTest, TreeKeyTypes);


TYPED_TEST(BPTreeTest, PseudoRandomInsert) {
    for (typename TestFixture::KeyType i = 1; i < TEST_MAX_KEY; i++) {
        typename TestFixture::KeyType value;
        ASSERT_TRUE(this->tree.search(i, value));
        EXPECT_EQ(i, value);
    }
}

TYPED_TEST(BPTreeTest, CountKeyTest) {
    for (typename TestFixture::KeyType i = 1; i < TEST_MAX_KEY; i++) {
        size_t count = this->tree.count_key(i);
        EXPECT_EQ(NUM_DUPLICATE, count);
    }
}

TYPED_TEST(BPTreeTest, EmptySearchTest) {
    EXPECT_FALSE(this->tree.search_iter(TEST_MAX_KEY / 2).empty());
    EXPECT_TRUE(this->tree.search_iter(TEST_MAX_KEY + 100).empty());
    EXPECT_TRUE(this->tree.search_iter(0).empty());

    typename TestFixture::TestTree empty_tree;
    EXPECT_TRUE(empty_tree.search_range(0).empty());
}

TYPED_TEST(BPTreeTest, OutOfRangeTest) {
    EXPECT_EQ(1, *(this->tree.search_range(0).begin()));
    EXPECT_EQ(TEST_MAX_KEY, *(this->tree.search_range(TEST_MAX_KEY+100).begin()));
}
