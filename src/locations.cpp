#include "locations.h"

size_t read_locations(FILE* file, size_t count, Location* output) {
    size_t locations_read = 0;
    for (size_t i = 0; i < count; i++) {
        Location* loc = output + i;
        if(fscanf(file, "%u|%127[^\t\n] ", &loc->id, loc->name) == EOF) {
            break;
        }
        locations_read++;
    }
    return locations_read;
}
