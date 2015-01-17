#ifndef _LOCATIONS_H
#define _LOCATIONS_H

#include <cstdint>
#include <fstream>
#include <string>

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

struct DeltaFunction {
    BPTree<DeltaRange, uint32_t> ranges;
    uint32_t max;

    uint32_t evaluate(uint32_t const value) const;
    NIEdge apply(NIEdge const& edge) const;
};

struct DeltaniModeData{
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
