#ifndef BPTREE_H
#define BPTREE_H

#include <cstdint>
#include <iterator>
#include <stack>

template <class V, size_t BPTREE_MAX_KEYS = 8>
class BPTree {
private:
    enum BPNodeType {BP_INNER, BP_LEAF};

    struct BPNode;

    struct BPInnerNode {
        BPNode* pointers[BPTREE_MAX_KEYS+1];
    };

    struct BPLeafValues {
        V values[BPTREE_MAX_KEYS];
    };

    struct BPLeaf {
        BPLeafValues* leaf_values;
        BPNode* prev;
        BPNode* next;
    };

    struct BPNode {
        BPNodeType type;
        size_t num_keys;
        int32_t keys[BPTREE_MAX_KEYS];
        BPNode* parent;
        size_t parent_pos;
        union {
            BPInnerNode inner;
            BPLeaf leaf;
        };
    };

    BPNode* root_node;

    static BPNode* alloc_leaf() {
        BPNode* node = new BPNode();
        node->type = BP_LEAF;
        node->leaf.leaf_values = new BPLeafValues();
        return node;
    }

    static size_t search_in_node(BPNode* node, int32_t key) {
        size_t max_bound = node->num_keys;
        size_t min_bound = 0;
        while (max_bound != min_bound) {
            size_t index = min_bound + (max_bound - min_bound) / 2;
            if (key < node->keys[index]) {
                max_bound = index;
            } else {
                min_bound = index + 1;
            }
        };
        return max_bound;
    }

    size_t search_leaf(int32_t key, BPNode** leaf) {
        BPNode* node = root_node;
        while (true) {
            size_t index = search_in_node(node, key);
            if (node->type == BP_INNER) {
                node = node->inner.pointers[index];
            } else {
                *leaf = node;
                return index;
            }
        }
    }

public:
    class BPKeyIterator
    : public std::iterator<std::input_iterator_tag, V, size_t> {
    private:
        int32_t const key;
        BPNode const* current_node;
        size_t current_index;

    public:
        BPKeyIterator(int32_t key)
        : key(key), current_node(nullptr) {
        }

        BPKeyIterator(int32_t key, BPNode* node, size_t index)
        : key(key), current_node(node), current_index(index) {
        }

        bool operator ==(BPKeyIterator& it) {
            if (current_node == nullptr) {
                return key == it.key;
            } else {
                return (key == it.key) &&
                    (current_node == it.current_node) &&
                    (current_index == it.current_index);
            }
        }

        bool operator !=(BPKeyIterator& it) {
            return !(*this == it);
        }

        V& operator *() {
            return current_node->leaf.leaf_values->values[current_index];
        }

        BPKeyIterator& operator ++() {
            if (current_node != nullptr) {
                if (current_index == 0) {
                    current_node = current_node->leaf.prev;
                    if (current_node == nullptr) {
                        return *this;
                    }
                    current_index = current_node->num_keys - 1;
                } else {
                    current_index--;
                }
                if (current_node->keys[current_index] != key) {
                    current_node = nullptr;
                }
            }
            return *this;
        }
    };

    class BPKeyValues {
    private:
        int32_t const key;
        BPNode* const node;
        size_t const index;

    public:
        BPKeyValues(int32_t key)
        : key(key), node(nullptr), index(0) {
        }

        BPKeyValues(int32_t key, BPNode* node, size_t index)
        : key(key), node(node), index(index) {
        }
        BPKeyIterator begin() {
            if (node == nullptr) {
                return end();
            } else {
                return BPKeyIterator(key, node, index);
            }
        }

        BPKeyIterator end() {
            return BPKeyIterator(key);
        }
    };

    class BPRangeIterator
    : public std::iterator<std::bidirectional_iterator_tag, V, size_t> {
    private:
        BPNode* node;
        size_t index;
    public:
        BPRangeIterator()
        : node(nullptr), index(0) {
        }

        BPRangeIterator(BPNode* node, size_t index)
        : node(node), index(index) {
        }

        bool operator ==(BPRangeIterator& it) {
            if (node == nullptr) {
                return it.node == nullptr;
            } else {
                return (node == it.node) && (index == it.index);
            }
        }

        bool operator !=(BPRangeIterator& it) {
            return !(*this == it);
        }

        V& operator *() {
            return node->leaf.leaf_values->values[index];
        }

        BPRangeIterator& operator ++() {
            if (node != nullptr) {
                index++;
                if (index >= node->num_keys) {
                    node = node->leaf.next;
                    index = 0;
                }
            }
            return *this;
        }

        BPRangeIterator& operator --() {
            if (node != nullptr) {
                if (index == 0) {
                    node = node->leaf.prev;
                    index = node->num_keys - 1;
                } else {
                    index--;
                }
            }
            return *this;
        }
    };

    class BPKeyRange {
    private:
        BPNode* const node;
        size_t const index;
    public:
        BPKeyRange()
        : node(nullptr), index(0) {
        }

        BPKeyRange(BPNode* node, size_t index)
        : node(node), index(index) {
        }

        BPRangeIterator begin() {
            if (node == nullptr) {
                return end();
            } else {
                return BPRangeIterator(node, index);
            }
        }

        BPRangeIterator end() {
            return BPRangeIterator();
        }
    };

    BPTree() {
        root_node = alloc_leaf();
        root_node->num_keys = 0;
        root_node->parent = nullptr;
        root_node->parent_pos = 0;
        root_node->leaf.prev = nullptr;
        root_node->leaf.next = nullptr;
    }

    ~BPTree() {
        std::stack<BPNode*> nodes;
        nodes.push(root_node);
        while (!nodes.empty()) {
            BPNode* node = nodes.top();
            nodes.pop();
            if (node->type == BP_LEAF) {
                delete node->leaf.leaf_values;
            } else {
                for (size_t i=0; i <= node->num_keys; i++) {
                    nodes.push(node->inner.pointers[i]);
                }
            }
            delete node;
        }
    }

    bool search(int32_t key, V& data) {
        if (root_node->num_keys == 0) {
            return false;
        } else {
            BPNode* leaf;
            size_t index = search_leaf(key, &leaf) - 1;
            if (index >= leaf->num_keys) {
                return false;
            } else {
                bool is_key = key == leaf->keys[index];
                if (is_key) {
                    data = leaf->leaf.leaf_values->values[index];
                }
                return is_key;
            }
        }
    }

    BPKeyValues search_iter(int32_t key) {
        if (root_node->num_keys > 0) {
            BPNode* leaf;
            size_t index = search_leaf(key, &leaf) - 1;
            if (index < leaf->num_keys) {
                if (key == leaf->keys[index]) {
                    return BPKeyValues(key, leaf, index);
                }
            }
        }
        return BPKeyValues(key);
    }

    BPKeyRange search_range(int32_t key) {
        if (root_node->num_keys > 0) {
            BPNode* leaf;
            size_t index = search_leaf(key, &leaf);
            if (index == 0) {
                if (leaf->leaf.prev == nullptr) {
                    return BPKeyRange(leaf, 0);
                } else {
                    return BPKeyRange(leaf->leaf.prev, leaf->leaf.prev->num_keys - 1);
                }
            } else {
                return BPKeyRange(leaf, index - 1);
            }
        }
        return BPKeyRange();
    }

    size_t count_key(int32_t key) {
        BPKeyValues v = search_iter(key);
        return std::distance(std::begin(v), std::end(v));
    }

    void insert(int32_t key, V& value) {
        if (root_node->num_keys == 0) {
            root_node->keys[0] = key;
            root_node->leaf.leaf_values->values[0] = V(value);
            root_node->num_keys++;
        } else {
            BPNode* node;
            size_t index = search_leaf(key, &node);
            if (node->num_keys < BPTREE_MAX_KEYS) {
                // move all keys and values one to the right
                // to be able to insert the new key
                for (size_t i = node->num_keys; i > index; i--) {
                    node->keys[i] = node->keys[i-1];
                    node->leaf.leaf_values->values[i] = node->leaf.leaf_values->values[i-1];
                }
                node->keys[index] = key;
                node->leaf.leaf_values->values[index] = V(value);
                node->num_keys++;
            } else {
                int32_t last_key;
                V last_value;
                if (index >= BPTREE_MAX_KEYS) {
                    last_key = key;
                    last_value = value;
                } else {
                    last_key = node->keys[BPTREE_MAX_KEYS - 1];
                    last_value = node->leaf.leaf_values->values[BPTREE_MAX_KEYS - 1];
                    for (size_t i = BPTREE_MAX_KEYS - 1; i > index; i--) {
                        node->keys[i] = node->keys[i-1];
                        node->leaf.leaf_values->values[i] = node->leaf.leaf_values->values[i-1];
                    }
                    node->keys[index] = key;
                    node->leaf.leaf_values->values[index] = V(value);
                }
                BPNode* new_node = alloc_leaf();
                new_node->parent = node->parent;
                for (size_t i = BPTREE_MAX_KEYS / 2 + 1; i < BPTREE_MAX_KEYS; i++) {
                    size_t j = i - BPTREE_MAX_KEYS / 2 - 1;
                    new_node->keys[j] = node->keys[i];
                    new_node->leaf.leaf_values->values[j] = node->leaf.leaf_values->values[i];
                }
                new_node->keys[BPTREE_MAX_KEYS / 2 - 1] = last_key;
                new_node->leaf.leaf_values->values[BPTREE_MAX_KEYS / 2 - 1] = last_value;
                new_node->num_keys = BPTREE_MAX_KEYS / 2;
                new_node->leaf.prev = node;
                new_node->leaf.next = node->leaf.next;
                node->num_keys = BPTREE_MAX_KEYS / 2 + 1;
                if (node->leaf.next != nullptr) {
                    node->leaf.next->leaf.prev = new_node;
                }
                node->leaf.next = new_node;
                BPNode* right_pointer = new_node;
                int32_t insert_key = new_node->keys[0];
                size_t insert_pos = node->parent_pos;
                node = node->parent;
                while (right_pointer != nullptr) {
                    if (node == nullptr) {
                        // create new root
                        BPNode* new_root = new BPNode();
                        new_root->type = BP_INNER;
                        new_root->parent = nullptr;
                        new_root->parent_pos = 0;
                        new_root->num_keys = 1;
                        new_root->keys[0] = insert_key;
                        new_root->inner.pointers[0] = root_node;
                        new_root->inner.pointers[1] = right_pointer;
                        root_node->parent = new_root;
                        right_pointer->parent = new_root;
                        right_pointer->parent_pos = 1;
                        root_node = new_root;
                        right_pointer = nullptr;
                    } else if (node->num_keys < BPTREE_MAX_KEYS) {
                        // just insert key and pointer in inner node
                        for (size_t i = node->num_keys; i > insert_pos; i--) {
                            node->keys[i] = node->keys[i-1];
                            BPNode* moved_node = node->inner.pointers[i];
                            moved_node->parent_pos++;
                            node->inner.pointers[i+1] = moved_node;
                        }
                        node->keys[insert_pos] = insert_key;
                        node->inner.pointers[insert_pos+1] = right_pointer;
                        node->num_keys++;
                        right_pointer->parent_pos = insert_pos + 1;
                        right_pointer = nullptr;
                    } else {
                        // insert key and pointer in inner node and let the
                        // middle element be propagated
                        BPNode* last_pointer;
                        if (insert_pos >= BPTREE_MAX_KEYS) {
                            last_key = insert_key;
                            last_pointer = right_pointer;
                        } else {
                            last_key = node->keys[BPTREE_MAX_KEYS - 1];
                            last_pointer = node->inner.pointers[BPTREE_MAX_KEYS];
                            for (size_t i = BPTREE_MAX_KEYS - 1; i > insert_pos; i--) {
                                node->keys[i] = node->keys[i-1];
                                BPNode* moved_node = node->inner.pointers[i];
                                moved_node->parent_pos++;
                                node->inner.pointers[i+1] = moved_node;
                            }
                            node->keys[insert_pos] = insert_key;
                            right_pointer->parent_pos = insert_pos + 1;
                            node->inner.pointers[insert_pos+1] = right_pointer;
                        }
                        new_node = new BPNode();
                        new_node->type = BP_INNER;
                        new_node->parent = node->parent;
                        new_node->inner.pointers[0] = node->inner.pointers[BPTREE_MAX_KEYS / 2 + 1];
                        new_node->inner.pointers[0]->parent = new_node;
                        new_node->inner.pointers[0]->parent_pos = 0;
                        for (size_t i = BPTREE_MAX_KEYS / 2 + 1; i < BPTREE_MAX_KEYS; i++) {
                            new_node->keys[i - BPTREE_MAX_KEYS / 2 - 1] = node->keys[i];
                            BPNode* moved_node = node->inner.pointers[i+1];
                            moved_node->parent = new_node;
                            size_t j = i - BPTREE_MAX_KEYS / 2;
                            moved_node->parent_pos = j;
                            new_node->inner.pointers[j] = moved_node;
                        }
                        new_node->keys[BPTREE_MAX_KEYS / 2 - 1] = last_key;
                        new_node->inner.pointers[BPTREE_MAX_KEYS / 2] = last_pointer;
                        last_pointer->parent = new_node;
                        last_pointer->parent_pos = BPTREE_MAX_KEYS / 2;
                        new_node->num_keys = BPTREE_MAX_KEYS / 2;
                        node->num_keys = BPTREE_MAX_KEYS / 2;
                        insert_key = node->keys[BPTREE_MAX_KEYS / 2];
                        right_pointer = new_node;
                        insert_pos = node->parent_pos;
                        node = node->parent;
                    }
                }
            }
        }
    }

    size_t depth() {
        size_t d = 1;
        BPNode* node = root_node;
        while (node->type != BP_LEAF) {
            d++;
            node = node->inner.pointers[0];
        }
        return d;
    }
};

#endif
