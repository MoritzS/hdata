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

size_t read_locations(FILE* file, size_t count, Location* output);
size_t read_adjacent_locations(FILE* file, size_t count, AdjacentLocation* output);

#endif
