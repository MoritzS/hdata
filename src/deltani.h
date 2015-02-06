#ifndef _DELTANI_H
#define _DELTANI_H

#include <cstdint>
#include <vector>

#include "bptree.h"
#include "nested_intervals.h"

class deltani_invalid_version
: public hierarchy_error {
public:
    virtual const char* what() const noexcept {
        return "deltani: invalid version";
    }
};

class deltani_invalid_key
: public hierarchy_error {
public:
    virtual const char* what() const noexcept {
        return "deltani: invalid key";
    }
};

class deltani_key_exists
: public deltani_invalid_key {
public:
    virtual const char* what() const noexcept {
        return "deltani: key already exists";
    }
};

class deltani_key_removed
: public deltani_invalid_key {
public:
    virtual const char* what() const noexcept {
        return "deltani: key is removed";
    }
};

class deltani_key_has_children
: public deltani_invalid_key {
public:
    virtual const char* what() const noexcept {
        return "deltani: key has children";
    }
};

template <
    class KeyType,
    class ValueType
>
class DeltaNI
: public Hierarchy<KeyType, ValueType> {
public:
    using NIEdge = typename NestedIntervals<KeyType, ValueType>::NIEdge;
    using NIEdgeTree = typename NestedIntervals<KeyType, ValueType>::NIEdgeTree;
    using ValueTree = typename Hierarchy<KeyType, ValueType>::ValueTree;

    struct DeltaRange {
        uint64_t from;
        uint64_t to;
    };

    typedef BPTree<DeltaRange, KeyType> DeltaRangeTree;

    class DeltaFunction {
    private:
        DeltaRangeTree ranges;
        DeltaRangeTree ranges_inv;

    public:
        uint64_t max;

        DeltaFunction()
        :ranges(), ranges_inv(), max(0) {
        }

        bool empty() const {
            return ranges.empty();
        }

        void add_range(DeltaRange const& range) {
            ranges.insert(range.from, range);
            ranges_inv.insert(range.to, range);
        }

        uint64_t evaluate(uint64_t const value) const {
            if (!ranges.empty()) {
                DeltaRange const& range = *ranges.search_range(value).begin();
                return value + range.to - range.from;
            } else {
                return value;
            }
        }

        uint64_t evaluate_inv(uint64_t const value) const {
            if (!ranges_inv.empty()) {
                DeltaRange const& range = *ranges_inv.search_range(value).begin();
                return value - range.to + range.from;
            } else {
                return value;
            }
        }

        NIEdge apply(NIEdge const& edge) const {
            NIEdge new_edge;
            new_edge.key = edge.key;
            new_edge.lower = evaluate(edge.lower);
            new_edge.upper = evaluate(edge.upper);
            return new_edge;
        }

        DeltaFunction merge(DeltaFunction const& delta) const {
            if (delta.ranges.empty()) {
                return *this;
            } else if (ranges.empty()) {
                return delta;
            }
            DeltaFunction new_delta;
            new_delta.max = delta.max;
            for (DeltaRange range : ranges) {
                new_delta.add_range({range.from, delta.evaluate(range.to)});
            }
            for (DeltaRange range : delta.ranges) {
                uint64_t from = evaluate_inv(range.from);
                if (new_delta.ranges.count_key(from) == 0) {
                    new_delta.add_range({from, range.to});
                }
            }
            return new_delta;
        }
    };

private:
    uint64_t init_max;
    uint64_t max_edge;
    NIEdgeTree edges;
    std::vector<std::vector<DeltaFunction>> deltas;
    DeltaFunction wip_delta;

    NIEdge get_edge(NIEdge const& edge, size_t const version, bool const use_wip) const {
        size_t v;
        if (version > max_version()) {
            v = max_version();
        } else {
            v = version;
        }
        if (v == 0) {
            if (wip_delta.empty()) {
                return edge;
            } else {
                return wip_delta.apply(edge);
            }
        }

        // power = floor(log2(version));
        size_t power = sizeof(size_t) * 8 - 1;
        while (v >> power == 0) {
            power--;
        }

        NIEdge new_edge = edge;
        size_t current_version = 0;
        while (current_version < v) {
            size_t step = UINTMAX_C(1) << power;
            new_edge = deltas[power][current_version / step].apply(new_edge);
            current_version += step;
            if (power > 0) {
                power--;
                while (((v >> power) & 1) == 0) {
                    power--;
                }
            }
        };

        if (use_wip) {
            new_edge = wip_delta.apply(new_edge);
        }

        return new_edge;
    }

    bool exists(KeyType const key, size_t const version, bool const use_wip) const {
        NIEdge edge;
        if (!edges.search(key, edge)) {
            return false;
        }
        if (version == 0) {
            if (use_wip && !wip_delta.empty()) {
                return wip_delta.evaluate(edge.lower) < wip_delta.max;
            } else {
                return edge.lower < init_max;
            }
        } else {
            edge = get_edge(edge, version, use_wip);
            if (use_wip && !wip_delta.empty()) {
                return edge.lower < wip_delta.max;
            } else {
                return edge.lower < deltas[0][version - 1].max;
            }
        }
    }

    bool is_ancestor(KeyType const parent, KeyType const child, size_t const version, bool const use_wip) const {
        NIEdge parent_edge;
        NIEdge child_edge;
        if (!edges.search(parent, parent_edge)) {
            throw deltani_invalid_key();
        }
        if (!edges.search(child, child_edge)) {
            throw deltani_invalid_key();
        }
        parent_edge = get_edge(parent_edge, version, use_wip);
        child_edge = get_edge(child_edge, version, use_wip);
        return parent_edge.lower < child_edge.lower && parent_edge.upper > child_edge.upper;
    }

public:
    DeltaNI()
    : Hierarchy<KeyType, ValueType>(), init_max(0), max_edge(0), edges(), deltas(), wip_delta() {
    }

    DeltaNI(ValueTree values, NIEdgeTree edges)
    : Hierarchy<KeyType, ValueType>(values), max_edge(0), edges(edges), deltas(), wip_delta() {
        // search root (edge with e.lower == 1)
        for (NIEdge& e : this->edges) {
            if (e.lower == 1) {
                init_max = e.upper + 1;
            }
            if (e.upper > max_edge) {
                max_edge = e.upper;
            }
        }
    }

    DeltaNI(ValueTree values, NIEdgeTree edges, uint64_t const max, uint64_t const max_edge)
    : Hierarchy<KeyType, ValueType>(values), init_max(max), max_edge(max_edge), edges(edges), deltas(), wip_delta() {
    }

    size_t max_version() const {
        if (deltas.empty()) {
            return 0;
        } else {
            return deltas[0].size();
        }
    }

    NIEdge get_edge(NIEdge const& edge) const {
        return get_edge(edge, max_version(), true);
    }

    NIEdge get_edge(NIEdge const& edge, size_t const version) const {
        if (version > max_version()) {
            throw deltani_invalid_version();
        }
        return get_edge(edge, version, false);
    }

    size_t insert_delta(DeltaFunction const& delta) {
        if (deltas.empty()) {
            deltas.emplace_back();
            deltas[0].push_back(delta);
            return 1;
        } else {
            deltas[0].push_back(delta);
            size_t size = deltas[0].size();
            for (size_t level = 0; size % 2 == 0; level++) {
                if (level + 1 >= deltas.size()) {
                    deltas.emplace_back();
                }
                DeltaFunction merged = deltas[level][size-2].merge(deltas[level][size-1]);
                deltas[level + 1].push_back(merged);
                size = deltas[level + 1].size();
            }
            return deltas[0].size();
        }
    }

    virtual bool exists(KeyType const key) const {
        return exists(key, max_version(), true);
    }

    virtual bool exists(KeyType const key, size_t const version) const {
        if (version > max_version()) {
            throw deltani_invalid_version();
        }
        return exists(key, version, false);
    }

    virtual size_t num_childs(KeyType const key, size_t const version) const {
        return 0;
    }

    virtual std::vector<KeyType> children(KeyType const key, size_t const version) const {
        return std::vector<KeyType>();
    }

    virtual bool is_ancestor(KeyType const parent, KeyType const child) const {
        return is_ancestor(parent, child, max_version(), true);
    }

    virtual bool is_ancestor(KeyType const parent, KeyType const child, size_t const version) const {
        if (version > max_version()) {
            throw deltani_invalid_version();
        }
        return is_ancestor(parent, child, version, false);
    }

    virtual void insert(KeyType const parent, KeyType const key, ValueType const value) {
        NIEdge parent_edge;
        if (!edges.search(parent, parent_edge)) {
            throw deltani_invalid_key();
        }
        if (!exists(parent)) {
            throw deltani_key_removed();
        }
        parent_edge = get_edge(parent_edge);

        if (exists(key)) {
            throw deltani_key_exists();
        }

        NIEdge inserting_edge;
        if (edges.search(key, inserting_edge)) {
            inserting_edge = get_edge(inserting_edge);
        } else {
            inserting_edge.key = key;
            inserting_edge.lower = max_edge + 1;
            inserting_edge.upper = max_edge + 2;
            edges.insert(key, inserting_edge);
            Hierarchy<KeyType, ValueType>::values.insert(key, value);
            max_edge += 2;
        }
        DeltaFunction delta;
        delta.add_range({1, 1});
        delta.add_range({parent_edge.upper, parent_edge.upper + 2});
        delta.add_range({inserting_edge.lower, parent_edge.upper});
        delta.add_range({inserting_edge.upper + 1, inserting_edge.upper + 1});
        if (!wip_delta.empty()) {
            delta.max = wip_delta.max + 2;
        } else if (deltas.empty()) {
            delta.max = init_max + 2;
        } else {
            delta.max = deltas[0][deltas[0].size() - 1].max + 2;
        }

        wip_delta = wip_delta.merge(delta);
    }

    virtual void remove(KeyType const key) {
        NIEdge edge;
        if (!edges.search(key, edge)) {
            throw deltani_invalid_key();
        }
        if (!exists(key)) {
            throw deltani_key_removed();
        }
        edge = get_edge(edge);

        if (edge.upper - edge.lower > 1) {
            throw deltani_key_has_children();
        }

        DeltaFunction delta;
        delta.add_range({1, 1});
        if (edge.lower == 1) {
            delta.max = 1;
        } else {
            if (!wip_delta.empty()) {
                delta.max = wip_delta.max - 2;
            } else if (deltas.empty()) {
                delta.max = init_max - 2;
            } else {
                delta.max = deltas[0][deltas[0].size() - 1].max - 2;
            }
            delta.add_range({edge.lower, delta.max});
            delta.add_range({edge.upper + 1, edge.lower});
            delta.add_range({delta.max + 2, delta.max + 2});
        }

        wip_delta = wip_delta.merge(delta);
    }

    virtual size_t commit() {
        if (wip_delta.empty()) {
            return max_version();
        } else {
            size_t new_version = insert_delta(wip_delta);
            wip_delta = DeltaFunction();
            return new_version;
        }
    }
};

#endif
