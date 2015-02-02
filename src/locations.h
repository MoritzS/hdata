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

    DeltaFunction();

    bool empty() const;

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

class deltani_id_exists
: public deltani_invalid_id {
public:
    using deltani_invalid_id::deltani_invalid_id;

    virtual const char* what() const noexcept {
        return "deltani: id already exists";
    }
};

class DeltaVersions {
private:
    uint32_t init_max;
    uint32_t max_edge;
    NIEdgeTree edges;
    std::vector<std::vector<DeltaFunction>> deltas;
    DeltaFunction wip_delta;

    NIEdge get_edge(NIEdge const& edge, size_t const version, bool const use_wip) const;
    bool exists(uint32_t const id, size_t const version, bool const use_wip) const;
    bool is_ancestor(uint32_t const parent_id, uint32_t const child_id, size_t const version, bool const use_wip) const;

public:
    DeltaVersions();
    DeltaVersions(NIEdgeTree& edges);
    DeltaVersions(NIEdgeTree& edges, uint32_t const max, uint32_t const max_edge);

    size_t max_version() const;

    NIEdge get_edge(NIEdge const& edge) const;
    NIEdge get_edge(NIEdge const& edge, size_t const version) const;
    size_t insert_delta(DeltaFunction const& delta);

    bool exists(uint32_t const id) const;
    bool exists(uint32_t const id, size_t const version) const;
    bool is_ancestor(uint32_t const parent_id, uint32_t const child_id) const;
    bool is_ancestor(uint32_t const parent_id, uint32_t const child_id, size_t const version) const;

    void insert(uint32_t const id, uint32_t const parent_id);
    uint32_t save();
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
