#include <cstdlib>
#include <cstdio>
#include "bptree.h"

size_t BPTree::search_in_node(BPNode* node, int32_t key) {
#ifdef BPTREE_USE_AVX2
    __m256i keys_sse = _mm256_load_si256((__m256i*) node->keys);
    __m256i search_key = _mm256_set1_epi32(key);
    keys_sse = _mm256_cmpgt_epi32(keys_sse, search_key);
    uint32_t mask = _mm256_movemask_epi8(keys_sse);
    mask |= 0xffffffff << (node->num_keys * 4);
    uint32_t count = _mm_popcnt_u32(mask) / 4;
    return BPTREE_MAX_KEYS - count;
#elif defined(BPTREE_USE_SSE2)
    __m128i keys_sse = _mm_load_si128((__m128i*) node->keys);
    __m128i search_key = _mm_set1_epi32(key);
    keys_sse = _mm_cmpgt_epi32(keys_sse, search_key);
    uint16_t mask = _mm_movemask_epi8(keys_sse);
    // mask now looks like this:
    // 0b1111111100000000
    // It has 16 bits.
    // The first n*4 bits are 1 where n is the number of keys in the node that
    // are lower than the search key.
    //
    // Because a node could also have less than 4 keys, we need to make sure that
    // all keys beyond the bounds of node->num_keys are considered greater than
    // the search key. i.e. the first n*4 bits have to be 1, where n is
    // node->num_keys.
    mask |= 0xffff << (node->num_keys * 4);
    // count how many keys are smaller than the search key
    if (mask == 0) {
        return node->num_keys;
    } else {
        return __tzcnt_u16(mask) / 4;
    }
#else
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
#endif
}

size_t BPTree::search_leaf(int32_t key, BPNode** leaf) {
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

BPTree::BPTree() {
    root_node = (BPNode*) malloc(sizeof(BPNode));
    root_node->type = BP_LEAF;
    root_node->num_keys = 0;
    root_node->parent = nullptr;
    root_node->leaf.prev = nullptr;
    root_node->leaf.next = nullptr;
}

bool BPTree::search(int32_t key, void** data) {
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
                *data = leaf->leaf.values[index];
            }
            return is_key;
        }
    }
}

void BPTree::insert(int32_t key, void* value) {
    if (root_node->num_keys == 0) {
        root_node->keys[0] = key;
        root_node->leaf.values[0] = value;
        root_node->num_keys++;
    } else {
        BPNode* node;
        size_t index = search_leaf(key, &node);
        if (node->num_keys < BPTREE_MAX_KEYS) {
            // move all keys and values one to the right
            // to be able to insert the new key
            for (size_t i = node->num_keys; i > index; i--) {
                node->keys[i] = node->keys[i-1];
                node->leaf.values[i] = node->leaf.values[i-1];
            }
            node->keys[index] = key;
            node->leaf.values[index] = value;
            node->num_keys++;
        } else {
            int32_t last_key;
            void* last_value;
            if (index >= BPTREE_MAX_KEYS) {
                last_key = key;
                last_value = value;
            } else {
                last_key = node->keys[BPTREE_MAX_KEYS - 1];
                last_value = node->leaf.values[BPTREE_MAX_KEYS - 1];
                for (size_t i = BPTREE_MAX_KEYS - 1; i > index; i--) {
                    node->keys[i] = node->keys[i-1];
                    node->leaf.values[i] = node->leaf.values[i-1];
                }
                node->keys[index] = key;
                node->leaf.values[index] = value;
            }
            BPNode* new_node = (BPNode*)malloc(sizeof(BPNode));
            new_node->type = BP_LEAF;
            new_node->parent = node->parent;
            for (size_t i = BPTREE_MAX_KEYS / 2; i < BPTREE_MAX_KEYS; i++) {
                new_node->keys[i - BPTREE_MAX_KEYS / 2] = node->keys[i];
                new_node->leaf.values[i - BPTREE_MAX_KEYS / 2] = node->leaf.values[i];
            }
            new_node->keys[BPTREE_MAX_KEYS / 2] = last_key;
            new_node->leaf.values[BPTREE_MAX_KEYS / 2] = last_value;
            new_node->num_keys = BPTREE_MAX_KEYS / 2 + 1;
            new_node->leaf.prev = node;
            new_node->leaf.next = node->leaf.next;
            node->num_keys = BPTREE_MAX_KEYS / 2;
            node->leaf.next = new_node;
            BPNode* right_pointer = new_node;
            int32_t insert_key = new_node->keys[0];
            node = node->parent;
            while (right_pointer != nullptr) {
                if (node == nullptr) {
                    // create new root
                    BPNode* new_root = (BPNode*)malloc(sizeof(BPNode));
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
                    new_node = (BPNode*)malloc(sizeof(BPNode));
                    new_node->type = BP_INNER;
                    new_node->parent = node->parent;
                    new_node->inner.pointers[0] = node->inner.pointers[BPTREE_MAX_KEYS / 2 + 1];
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

size_t BPTree::depth() {
    size_t d = 1;
    BPNode* node = root_node;
    while (node->type != BP_LEAF) {
        d++;
        node = node->inner.pointers[0];
    }
    return d;
}

void BPTree::draw_tree() {
    printf("(");
    for (size_t i=0; i<root_node->num_keys; i++) {
        printf(" %i", root_node->keys[i]);
    }
    printf(" )\n");
    size_t d = depth();
    if (d > 1) {
        for (size_t i=0; i<=root_node->num_keys; i++) {
            printf("(");
            BPNode* node = root_node->inner.pointers[i];
            for (size_t j=0; j<node->num_keys; j++) {
                printf(" %i", node->keys[j]);
            }
            printf(" ) ");
        }
        printf("\n");
    }
    if (d > 2) {
        for (size_t i=0; i<=root_node->num_keys; i++) {
            BPNode *node1 = root_node->inner.pointers[i];
            for (size_t j=0; j<=node1->num_keys; j++) {
                printf("(");
                BPNode *node = node1->inner.pointers[j];
                for (size_t k=0; k<node->num_keys; k++) {
                    printf(" %i", node->keys[k]);
                }
                printf(" ) ");
            }
            printf("|| ");
        }
        printf("\n");
    }
}
