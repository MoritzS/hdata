#ifndef BPTREE_H
#define BPTREE_H

#include <config.h>
#include <cstddef>
#include <cstdint>

#include "sse_util.h"

#if !(defined(BPTREE_USE_AVX2) || defined(BPTREE_USE_SSE2) || defined (BPTREE_NO_SSE))

    #ifdef HAVE_AVX2
        #define BPTREE_USE_AVX2
    #elif defined(HAVE_SSE2)
        #define BPTREE_USE_SSE2
    #endif

#endif

#ifdef BPTREE_USE_AVX2

    #ifndef HAVE_AVX2
        #error "requested AVX2 is not available"
    #endif

    #include <x86intrin.h>
    #define BPTREE_MAX_KEYS 8
    #define __BP_aligned __aligned256

#elif defined(BPTREE_USE_SSE2)

    #ifndef HAVE_SSE2
        #error "requested SSE2 is not available"
    #endif

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
