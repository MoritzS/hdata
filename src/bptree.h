#ifndef BPTREE_H
#define BPTREE_H

#include <cstddef>
#include <cstdint>

#include "sse_util.h"

#ifdef BPTREE_USE_AVX2

#include <x86intrin.h>
#define BPTREE_MAX_KEYS 8
#define __BP_aligned __aligned256

#elif BPTREE_USE_SSE2

#include <x86intrin.h>
#define BPTREE_MAX_KEYS 4
#define __BP_aligned __aligned128

#else

#ifndef BPTREE_MAX_KEYS
#define BPTREE_MAX_KEYS 8
#endif
#define __BP_aligned

#endif

class BPTree {
private:
    enum BPNodeType {BP_INNER, BP_LEAF};

    struct BPNode;

    struct BPInnerNode {
        BPNode* pointers[BPTREE_MAX_KEYS+1];
    };

    struct BPLeaf {
        void* values[BPTREE_MAX_KEYS];
        BPNode* prev;
        BPNode* next;
    };

    struct BPNode {
        BPNodeType type;
        size_t num_keys;
        int32_t keys[BPTREE_MAX_KEYS] __BP_aligned;
        BPNode* parent;
        union {
            BPInnerNode inner;
            BPLeaf leaf;
        };
    };

    BPNode* root_node;

    static size_t search_in_node(BPNode* node, int32_t key);
    size_t search_leaf(int32_t key, BPNode** leaf);
    void insert_into_inner_node(BPNode* node, int32_t key, BPNode* pointer);

public:
    BPTree();
    bool search(int32_t key, void** data);
    void insert(int32_t key, void* value);
    size_t depth();

    void draw_tree();
};

#endif
