#ifndef _LOCATIONS_H
#define _LOCATIONS_H

#include <cstdint>
#include <exception>
#include <fstream>
#include <string>
#include <vector>

#include "bptree.h"

struct Location {
    uint32_t id;
    char name[128];
};

struct AdjacentLocation {
    uint32_t parent_id;
    uint32_t child_id;
};

struct NIEdge {
    uint32_t loc_id;
    uint32_t lower;
    uint32_t upper;
};

typedef BPTree<Location, uint32_t> LocationTree;
typedef BPTree<AdjacentLocation, uint32_t> AdjLocTree;
typedef BPTree<NIEdge, uint32_t> NIEdgeTree;

size_t read_locations(std::ifstream& file, LocationTree& output);
size_t read_ni_edges(std::ifstream& file, BPTree<NIEdge>& output);

struct AdjModeData {
    AdjLocTree edges;
};

struct NiModeData {
    NIEdgeTree edges;
};

struct DeltaRange {
    uint32_t from;
    uint32_t to;
};

typedef BPTree<DeltaRange, uint32_t> DeltaRangeTree;

class DeltaFunction {
private:
    DeltaRangeTree ranges;
    DeltaRangeTree ranges_inv;

public:
    uint32_t max;

    void add_range(DeltaRange const& range);
    uint32_t evaluate(uint32_t const value) const;
    uint32_t evaluate_inv(uint32_t const value) const;
    NIEdge apply(NIEdge const& edge) const;
    DeltaFunction merge(DeltaFunction const& delta) const;
};

class deltani_error
: public std::exception {
public:
    virtual const char* what() const noexcept {
        return "deltani error";
    }
};

class deltani_invalid_id
: public deltani_error {
public:
    uint32_t const id;

    deltani_invalid_id(uint32_t const id)
    : id(id) {
    }

    virtual const char* what() const noexcept {
        return "deltani: invalid id";
    }
};

class DeltaVersions {
private:
    uint32_t init_max;
    NIEdgeTree edges;
    std::vector<std::vector<DeltaFunction>> deltas;

public:
    DeltaVersions();
    DeltaVersions(NIEdgeTree& edges);
    DeltaVersions(NIEdgeTree& edges, uint32_t const max);

    inline size_t max_version() const {
        return deltas[0].size();
    }

    NIEdge get_version(NIEdge const& edge, size_t version) const;
    size_t insert_delta(DeltaFunction const& delta);

    bool is_ancestor(uint32_t const parent_id, uint32_t child_id);
    bool is_ancestor(uint32_t const parent_id, uint32_t child_id, uint32_t version);
};

struct DeltaniModeData{
    DeltaVersions versions;
};

union ModeData {
    ModeData() {}
    AdjModeData* adj;
    NiModeData* ni;
    DeltaniModeData* deltani;
};

struct ModeInfo {
    int (*init_mode)(std::ifstream&, ModeData&);
    int (*run_input)(LocationTree&, ModeData&, std::string&);
    int (*exit_mode)(ModeData&);
};

extern ModeInfo adjModeInfo;
extern ModeInfo niModeInfo;
extern ModeInfo deltaniModeInfo;

#endif
