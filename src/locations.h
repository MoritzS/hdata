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

struct AdjModeData {
    BPTree<AdjacentLocation> edges;
};

struct NiModeData {
    BPTree<NIEdge> edges;
};

struct DeltaniModeData{
};

union ModeData {
    ModeData() {}
    AdjModeData adj;
    NiModeData ni;
    DeltaniModeData deltani;
};

struct ModeInfo {
    int (*init_mode)(std::ifstream&, ModeData&);
    int (*run_input)(BPTree<Location>&, ModeData&, std::string&);
    int (*exit_mode)(BPTree<Location>&, ModeData&);
};

extern ModeInfo adjModeInfo;
extern ModeInfo niModeInfo;
extern ModeInfo deltaniModeInfo;

size_t read_locations(std::ifstream& file, BPTree<Location>& output);

#endif
