#ifndef _ADJ_LIST_H
#define _ADJ_LIST_H

#include <stack>
#include <vector>

#include "bptree.h"
#include "hierarchy.h"

template <
    class KeyType,
    class ValueType
>
class AdjacencyList
: public Hierarchy<KeyType, ValueType> {
public:
    struct AdjacentEdge {
        KeyType parent;
        KeyType child;
    };

    typedef BPTree<AdjacentEdge, KeyType> AdjacencyTree;
    using ValueTree = typename Hierarchy<KeyType, ValueType>::ValueTree;

private:
    AdjacencyTree edges;

public:
    AdjacencyList(ValueTree values, AdjacencyTree edges)
    : Hierarchy<KeyType, ValueType>(values), edges(edges) {
    }

    AdjacencyList()
    : Hierarchy<KeyType, ValueType>(), edges() {
    }

    virtual bool exists(KeyType const key, size_t const version) const {
        return Hierarchy<KeyType, ValueType>::values.count_key(key) > 0;
    }

    virtual size_t num_childs(KeyType const key, size_t const version) const {
        if (Hierarchy<KeyType, ValueType>::values.count_key(key) == 0) {
            throw hierarchy_key_not_found();
        }
        return edges.count_key(key);
    }

    virtual std::vector<KeyType> children(KeyType const key, size_t const version) const {
        if (Hierarchy<KeyType, ValueType>::values.count_key(key) == 0) {
            throw hierarchy_key_not_found();
        }
        std::vector<KeyType> child_keys;
        for (AdjacentEdge& edge : edges.search_iter(key)) {
            child_keys.push_back(edge.child);
        }
        return child_keys;
    }

    virtual bool is_ancestor(KeyType const parent, KeyType const child, size_t const version) const {
        std::stack<KeyType> dfs_stack;
        dfs_stack.push(parent);
        while (!dfs_stack.empty()) {
            KeyType key = dfs_stack.top();
            dfs_stack.pop();
            for (AdjacentEdge& edge : edges.search_iter(key)) {
                if (edge.child == child) {
                    return true;
                }
                dfs_stack.push(edge.child);
            }
        }
        return false;
    }

    virtual void insert(KeyType const parent, KeyType const key, ValueType const value) {
        return;
    }

    virtual void remove(KeyType const key) {
        return;
    }

    virtual size_t commit() {
        return 0;
    }
};

#endif
