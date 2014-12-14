#ifndef BPTREE_H
#define BPTREE_H

#include <cstdint>
#include <cstring>
#include <iterator>
#include <stack>

template <class ValueType, class KeyType = int32_t, size_t MAX_KEYS = 8>
class BPTree {
private:
    enum BPNodeType {BP_INNER, BP_LEAF};

    struct BPNode;

    struct BPInnerNode {
        BPNode* pointers[MAX_KEYS+1];
    };

    struct BPLeafValues {
        ValueType values[MAX_KEYS];
    };

    struct BPLeaf {
        BPLeafValues* leaf_values;
        BPNode* prev;
        BPNode* next;
    };

    struct BPNode {
        BPNodeType type;
        size_t num_keys;
        KeyType keys[MAX_KEYS];
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

    static size_t search_in_node(BPNode* node, KeyType const key) {
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

    size_t search_leaf(KeyType const key, BPNode*& leaf) const {
        BPNode* node = root_node;
        while (true) {
            size_t index = search_in_node(node, key);
            if (node->type == BP_INNER) {
                node = node->inner.pointers[index];
            } else {
                leaf = node;
                return index;
            }
        }
    }

    static void move_keys(BPNode* node, size_t const from) {
        if (from + 1 < MAX_KEYS) {
            size_t num_moving;
            if (node->num_keys < MAX_KEYS) {
                num_moving = node->num_keys - from;
            } else {
                num_moving = node->num_keys - from - 1;
            }
            memmove(node->keys + from + 1, node->keys + from, sizeof(KeyType) * num_moving);
        }
    }

    static void move_values(BPNode* node, size_t const from) {
        if (from + 1 < MAX_KEYS) {
            size_t num_moving;
            if (node->num_keys < MAX_KEYS) {
                num_moving = node->num_keys - from;
            } else {
                num_moving = node->num_keys - from - 1;
            }
            memmove(
                node->leaf.leaf_values->values + from + 1,
                node->leaf.leaf_values->values + from,
                sizeof(ValueType) * num_moving
            );
        }
    }

    static void move_pointers(BPNode* node, size_t const from) {
        if (from < MAX_KEYS) {
            size_t num_moving;
            if (node->num_keys < MAX_KEYS) {
                num_moving = node->num_keys + 1 - from;
            } else {
                num_moving = node->num_keys + 1 - from - 1;
            }
            memmove(
                node->inner.pointers + from + 1,
                node->inner.pointers + from,
                sizeof(BPNode*) * num_moving
            );
        }
    }

    static void copy_keys(BPNode* node, BPNode* new_node) {
        memcpy(
            new_node->keys,
            node->keys + MAX_KEYS / 2 + 1,
            sizeof(KeyType*) * (MAX_KEYS / 2 - 1)
        );
    }

    static void copy_values(BPNode* node, BPNode* new_node) {
        memcpy(
            new_node->leaf.leaf_values->values,
            node->leaf.leaf_values->values + MAX_KEYS / 2 + 1,
            sizeof(ValueType) * (MAX_KEYS / 2 - 1)
        );
    }

    static void copy_pointers(BPNode* node, BPNode* new_node) {
        memcpy(
            new_node->inner.pointers + 1,
            node->inner.pointers + MAX_KEYS / 2 + 2,
            sizeof(BPNode*) * (MAX_KEYS / 2 - 1)
        );
    }

    bool insert_leaf(
        KeyType const key,
        ValueType const& value,
        KeyType& overflow_key,
        size_t& insert_pos,
        BPNode*& parent_node,
        BPNode*& created_node
    ) {
        if (root_node->num_keys == 0) {
            root_node->keys[0] = key;
            root_node->leaf.leaf_values->values[0] = ValueType(value);
            root_node->num_keys++;
            return false;
        } else {
            BPNode* node;
            size_t index = search_leaf(key, node);
            if (node->num_keys < MAX_KEYS) {
                // insert key and value at index
                move_keys(node, index);
                move_values(node, index);
                node->keys[index] = key;
                node->leaf.leaf_values->values[index] = ValueType(value);
                node->num_keys++;
                return false;
            } else {
                // insert key and value at index but save the last key and value
                // because they will be overwritten by move_keys and move_values
                KeyType last_key;
                ValueType last_value;
                // if new key's insert index is greater than MAX_KEYS it simply is
                // the last value
                if (index >= MAX_KEYS) {
                    last_key = key;
                    last_value = value;
                } else {
                    last_key = node->keys[MAX_KEYS - 1];
                    last_value = node->leaf.leaf_values->values[MAX_KEYS - 1];
                    move_keys(node, index);
                    move_values(node, index);
                    node->keys[index] = key;
                    node->leaf.leaf_values->values[index] = ValueType(value);
                }
                // copy the last MAX_KEYS / 2 - 1 keys and values from node to
                // a new node
                BPNode* new_node = alloc_leaf();
                new_node->parent = node->parent;
                copy_keys(node, new_node);
                copy_values(node, new_node);
                // insert last values at the end of the new node
                new_node->keys[MAX_KEYS / 2 - 1] = last_key;
                new_node->leaf.leaf_values->values[MAX_KEYS / 2 - 1] = last_value;
                new_node->num_keys = MAX_KEYS / 2;
                // update prev and next pointers
                new_node->leaf.prev = node;
                new_node->leaf.next = node->leaf.next;
                node->num_keys = MAX_KEYS / 2 + 1;
                if (node->leaf.next != nullptr) {
                    node->leaf.next->leaf.prev = new_node;
                }
                node->leaf.next = new_node;

                overflow_key = new_node->keys[0];
                insert_pos = node->parent_pos;
                parent_node = node->parent;
                created_node = new_node;
                return true;
            }
        }
    }

    bool insert_inner(
        KeyType& insert_key,
        size_t& insert_pos,
        BPNode*& node,
        BPNode*& created_node
    ) {
        if (node == nullptr) {
            // create new root
            BPNode* new_root = new BPNode();
            new_root->type = BP_INNER;
            new_root->parent = nullptr;
            new_root->parent_pos = 0;
            new_root->num_keys = 1;
            new_root->keys[0] = insert_key;
            new_root->inner.pointers[0] = root_node;
            new_root->inner.pointers[1] = created_node;
            root_node->parent = new_root;
            created_node->parent = new_root;
            created_node->parent_pos = 1;
            root_node = new_root;
            return false;
        } else if (node->num_keys < MAX_KEYS) {
            // just insert key and pointer in inner node
            for (size_t i = insert_pos; i < node->num_keys; i++) {
                node->inner.pointers[i + 1]->parent_pos++;
            }
            move_keys(node, insert_pos);
            move_pointers(node, insert_pos + 1);
            node->keys[insert_pos] = insert_key;
            node->inner.pointers[insert_pos+1] = created_node;
            node->num_keys++;
            created_node->parent_pos = insert_pos + 1;
            return false;
        } else {
            // insert key and pointer in inner node but save last key and
            // pointer like in insert_leaf
            KeyType last_key;
            BPNode* last_pointer;
            if (insert_pos >= MAX_KEYS) {
                last_key = insert_key;
                last_pointer = created_node;
            } else {
                last_key = node->keys[MAX_KEYS - 1];
                last_pointer = node->inner.pointers[MAX_KEYS];
                // moved nodes need to have their parent_pos updated
                for (size_t i = insert_pos; i < node->num_keys; i++) {
                    node->inner.pointers[i + 1]->parent_pos++;
                }
                move_keys(node, insert_pos);
                move_pointers(node, insert_pos + 1);
                node->keys[insert_pos] = insert_key;
                created_node->parent_pos = insert_pos + 1;
                node->inner.pointers[insert_pos+1] = created_node;
            }
            BPNode* new_node = new BPNode();
            new_node->type = BP_INNER;
            new_node->parent = node->parent;
            new_node->inner.pointers[0] = node->inner.pointers[MAX_KEYS / 2 + 1];
            copy_keys(node, new_node);
            copy_pointers(node, new_node);
            new_node->keys[MAX_KEYS / 2 - 1] = last_key;
            new_node->inner.pointers[MAX_KEYS / 2] = last_pointer;
            new_node->num_keys = MAX_KEYS / 2;
            // pointers that were moved to the new_node need to have their
            // parent and parent_pos updated
            for (size_t i = 0; i <= MAX_KEYS / 2; i++) {
                BPNode* moved_node = new_node->inner.pointers[i];
                moved_node->parent = new_node;
                moved_node->parent_pos = i;
            }
            node->num_keys = MAX_KEYS / 2;

            insert_key = node->keys[MAX_KEYS / 2];
            insert_pos = node->parent_pos;
            node = node->parent;
            created_node = new_node;
            return true;
        }
    }


public:
    class BPKeyIterator
    : public std::iterator<std::input_iterator_tag, ValueType, size_t> {
    private:
        KeyType const key;
        BPNode const* current_node;
        size_t current_index;

    public:
        BPKeyIterator(KeyType const key)
        : key(key), current_node(nullptr) {
        }

        BPKeyIterator(KeyType const key, BPNode const* const node, size_t const index)
        : key(key), current_node(node), current_index(index) {
        }

        bool operator ==(BPKeyIterator const& it) const {
            if (current_node == nullptr) {
                return key == it.key;
            } else {
                return (key == it.key) &&
                    (current_node == it.current_node) &&
                    (current_index == it.current_index);
            }
        }

        bool operator !=(BPKeyIterator const& it) const {
            return !(*this == it);
        }

        ValueType& operator *() const {
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
        KeyType const key;
        BPNode const* const node;
        size_t const index;

    public:
        BPKeyValues(KeyType const key)
        : key(key), node(nullptr), index(0) {
        }

        BPKeyValues(KeyType const key, BPNode const* const node, size_t const index)
        : key(key), node(node), index(index) {
        }

        BPKeyIterator begin() const {
            if (node == nullptr) {
                return end();
            } else {
                return BPKeyIterator(key, node, index);
            }
        }

        BPKeyIterator end() const {
            return BPKeyIterator(key);
        }
    };

    class BPRangeIterator
    : public std::iterator<std::bidirectional_iterator_tag, ValueType, size_t> {
    private:
        BPNode const* node;
        size_t index;
    public:
        BPRangeIterator()
        : node(nullptr), index(0) {
        }

        BPRangeIterator(BPNode const* const node, size_t const index)
        : node(node), index(index) {
        }

        bool operator ==(BPRangeIterator const& it) const {
            if (node == nullptr) {
                return it.node == nullptr;
            } else {
                return (node == it.node) && (index == it.index);
            }
        }

        bool operator !=(BPRangeIterator const& it) const {
            return !(*this == it);
        }

        ValueType& operator *() const {
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
        BPNode const* const node;
        size_t const index;
    public:
        BPKeyRange()
        : node(nullptr), index(0) {
        }

        BPKeyRange(BPNode const* const node, size_t const index)
        : node(node), index(index) {
        }

        BPRangeIterator begin() const {
            if (node == nullptr) {
                return end();
            } else {
                return BPRangeIterator(node, index);
            }
        }

        BPRangeIterator end() const {
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

    bool search(KeyType const key, ValueType& data) const {
        if (root_node->num_keys == 0) {
            return false;
        } else {
            BPNode* leaf;
            size_t index = search_leaf(key, leaf) - 1;
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

    BPKeyValues search_iter(KeyType const key) const {
        if (root_node->num_keys > 0) {
            BPNode* leaf;
            size_t index = search_leaf(key, leaf) - 1;
            if (index < leaf->num_keys) {
                if (key == leaf->keys[index]) {
                    return BPKeyValues(key, leaf, index);
                }
            }
        }
        return BPKeyValues(key);
    }

    BPKeyRange search_range(KeyType const key) const {
        if (root_node->num_keys > 0) {
            BPNode* leaf;
            size_t index = search_leaf(key, leaf);
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

    size_t count_key(KeyType const key) const {
        BPKeyValues v = search_iter(key);
        return std::distance(std::begin(v), std::end(v));
    }

    void insert(KeyType const key, ValueType const& value) {
        KeyType overflow_key;
        size_t insert_pos;
        BPNode* parent_node;
        BPNode* created_node;

        if (insert_leaf(key, value, overflow_key, insert_pos, parent_node, created_node)) {
            // insert overflowing values into inner nodes as long as
            // an overflow occurs
            while (insert_inner(overflow_key, insert_pos, parent_node, created_node)) {
            }
        }
    }

    size_t depth() const {
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
