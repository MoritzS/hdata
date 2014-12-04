#include "locations.h"

size_t read_locations(FILE* file, size_t count, BPTree<Location>& tree) {
    size_t locations_read = 0;
    for (size_t i = 0; i < count; i++) {
        Location loc;
        if(fscanf(file, "%u|%127[^\t\n] ", &loc.id, loc.name) == EOF) {
            break;
        }
        tree.insert((int32_t)loc.id, loc);
        locations_read++;
    }
    return locations_read;
}

ModeInfo adjModeInfo = {
    nullptr,
    nullptr,
    nullptr
};
ModeInfo niModeInfo = {
    nullptr,
    nullptr,
    nullptr
};
ModeInfo deltaniModeInfo = {
    nullptr,
    nullptr,
    nullptr
};
