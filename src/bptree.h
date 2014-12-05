#ifndef BPTREE_H
#define BPTREE_H

#include <config.h>
#include <cstdint>

#ifndef BPTREE_MAX_KEYS
    #define BPTREE_MAX_KEYS 8
#endif


template <class V>
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
    BPTree() {
        root_node = alloc_leaf();
        root_node->num_keys = 0;
        root_node->parent = nullptr;
        root_node->leaf.prev = nullptr;
        root_node->leaf.next = nullptr;
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
                node->leaf.next = new_node;
                BPNode* right_pointer = new_node;
                int32_t insert_key = new_node->keys[0];
                node = node->parent;
                while (right_pointer != nullptr) {
                    if (node == nullptr) {
                        // create new root
                        BPNode* new_root = new BPNode();
                        new_root->type = BP_INNER;
                        new_root->parent = nullptr;
                        new_root->num_keys = 1;
                        new_root->keys[0] = insert_key;
                        new_root->inner.pointers[0] = root_node;
                        new_root->inner.pointers[1] = right_pointer;
                        root_node->parent = new_root;
                        right_pointer->parent = new_root;
                        root_node = new_root;
                        right_pointer = nullptr;
                    } else if (node->num_keys < BPTREE_MAX_KEYS) {
                        // just insert key and pointer in inner node
                        size_t index = search_in_node(node, insert_key);
                        for (size_t i = node->num_keys; i > index; i--) {
                            node->keys[i] = node->keys[i-1];
                            node->inner.pointers[i+1] = node->inner.pointers[i];
                        }
                        node->keys[index] = insert_key;
                        node->inner.pointers[index+1] = right_pointer;
                        node->num_keys++;
                        right_pointer = nullptr;
                    } else {
                        // insert key and pointer in inner node and let the
                        // middle element be propagated
                        index = search_in_node(node, insert_key);
                        BPNode* last_pointer;
                        if (index >= BPTREE_MAX_KEYS) {
                            last_key = insert_key;
                            last_pointer = right_pointer;
                        } else {
                            last_key = node->keys[BPTREE_MAX_KEYS - 1];
                            last_pointer = node->inner.pointers[BPTREE_MAX_KEYS];
                            for (size_t i = BPTREE_MAX_KEYS - 1; i > index; i--) {
                                node->keys[i] = node->keys[i-1];
                                node->inner.pointers[i+1] = node->inner.pointers[i];
                            }
                            node->keys[index] = insert_key;
                            node->inner.pointers[index+1] = right_pointer;
                        }
                        new_node = new BPNode();
                        new_node->type = BP_INNER;
                        new_node->parent = node->parent;
                        new_node->inner.pointers[0] = node->inner.pointers[BPTREE_MAX_KEYS / 2 + 1];
                        new_node->inner.pointers[0]->parent = new_node;
                        for (size_t i = BPTREE_MAX_KEYS / 2 + 1; i < BPTREE_MAX_KEYS; i++) {
                            new_node->keys[i - BPTREE_MAX_KEYS / 2 - 1] = node->keys[i];
                            BPNode* moved_node = node->inner.pointers[i+1];
                            moved_node->parent = new_node;
                            new_node->inner.pointers[i - BPTREE_MAX_KEYS / 2] = moved_node;
                        }
                        new_node->keys[BPTREE_MAX_KEYS / 2 - 1] = last_key;
                        new_node->inner.pointers[BPTREE_MAX_KEYS / 2] = last_pointer;
                        last_pointer->parent = new_node;
                        new_node->num_keys = BPTREE_MAX_KEYS / 2;
                        node->num_keys = BPTREE_MAX_KEYS / 2;
                        insert_key = node->keys[BPTREE_MAX_KEYS / 2];
                        right_pointer = new_node;
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
