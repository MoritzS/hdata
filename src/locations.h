#ifndef _LOCATIONS_H
#define _LOCATIONS_H

#include <cstdint>
#include <cstdio>
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

struct AdjModeData {
};

struct NiModeData {
};

struct DeltaniModeData{
};

union ModeData {
    AdjModeData adj;
    NiModeData ni;
    DeltaniModeData deltani;
};

struct ModeInfo {
    int (*init_mode)(FILE*, ModeData&);
    int (*run_input)(BPTree<Location>&, ModeData&, char*);
    int (*exit_mode)(BPTree<Location>&, ModeData&);
};

extern ModeInfo adjModeInfo;
extern ModeInfo niModeInfo;
extern ModeInfo deltaniModeInfo;

size_t read_locations(FILE* file, BPTree<Location>& output);

#endif
