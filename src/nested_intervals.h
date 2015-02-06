#ifndef _NESTED_INTERVALS_H
#define _NESTED_INTERVALS_H

#include <iostream>
#include <cstdint>
#include <vector>

#include "bptree.h"
#include "hierarchy.h"

template <
    class KeyType,
    class ValueType
>
class NestedIntervals
: public Hierarchy<KeyType, ValueType> {
public:
    struct NIEdge {
        KeyType key;
        uint64_t lower;
        uint64_t upper;
    };

    typedef BPTree<NIEdge, KeyType> NIEdgeTree;
    typedef BPTree<NIEdge, uint64_t> NISortedEdgeTree;
    using ValueTree = typename Hierarchy<KeyType, ValueType>::ValueTree;

private:
    NIEdgeTree edges;
    NISortedEdgeTree sorted_edges;

public:
    NestedIntervals(ValueTree values, NIEdgeTree edges, NISortedEdgeTree sorted_edges)
    : Hierarchy<KeyType, ValueType>(values), edges(edges), sorted_edges(sorted_edges) {
    }

    NestedIntervals(ValueTree values, NIEdgeTree edges)
    : Hierarchy<KeyType, ValueType>(values), edges(edges), sorted_edges() {
        for (NIEdge& edge : edges) {
            sorted_edges.insert(edge.lower, edge);
        }
    }

    NestedIntervals()
    : Hierarchy<KeyType, ValueType>(), edges() {
    }

    virtual bool exists(KeyType const key, size_t const version) const {
        return Hierarchy<KeyType, ValueType>::values.count_key(key) > 0;
    }

    virtual size_t num_childs(KeyType const key, size_t const version) const {
        size_t num = 0;
        NIEdge parent_edge;
        if (!edges.search(key, parent_edge)) {
            throw hierarchy_key_not_found();
        }
        uint64_t last = parent_edge.lower;
        for (NIEdge& edge : sorted_edges.search_range(key)) {
            if (edge.lower <= last) {
                continue;
            } else if (edge.lower > parent_edge.upper) {
                break;
            } else {
                last = edge.upper;
                num++;
            }
        }
        return num;
    }

    virtual std::vector<KeyType> children(KeyType const key, size_t const version) const {
        std::vector<KeyType> child_keys;
        NIEdge parent_edge;
        if (!edges.search(key, parent_edge)) {
            throw hierarchy_key_not_found();
        }
        uint64_t last = parent_edge.lower;
        for (NIEdge& edge : sorted_edges.search_range(key)) {
            if (edge.lower <= last) {
                continue;
            } else if (edge.lower > parent_edge.upper) {
                break;
            } else {
                last = edge.upper;
                child_keys.push_back(edge.key);
            }
        }
        return child_keys;
    }

    virtual bool is_ancestor(KeyType const parent, KeyType const child, size_t const version) const {
        NIEdge parent_edge;
        NIEdge child_edge;
        if (!edges.search(parent, parent_edge) || !edges.search(child, child_edge)) { 
            throw hierarchy_key_not_found();
        }
        return parent_edge.lower < child_edge.lower && parent_edge.upper > child_edge.upper;
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
