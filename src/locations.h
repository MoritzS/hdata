#ifndef _LOCATIONS_H
#define _LOCATIONS_H

#include <cstdint>
#include <cstdio>

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
    int (*init_mode)(Location*, FILE*, ModeData&);
    int (*run_input)(Location*, ModeData&, char*);
    int (*exit_mode)(Location*, ModeData&);
};

extern ModeInfo adjModeInfo;
extern ModeInfo niModeInfo;
extern ModeInfo deltaniModeInfo;

size_t read_locations(FILE* file, size_t count, Location* output);

#endif
