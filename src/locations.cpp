#include "locations.h"

#include <cstdio>


size_t read_locations(FILE* file, BPTree<Location>& tree) {
    size_t locations_read = 0;
    while(1) {
        Location loc;
        if (fscanf(file, "%u|%127[^\t\n] ", &loc.id, loc.name) == EOF) {
            break;
        }
        tree.insert((int32_t)loc.id, loc);
        locations_read++;
    }
    return locations_read;
}

ModeInfo adjModeInfo = {
    // init_mode
    [] (FILE* file, ModeData& data) {
        printf("reading edges... ");
        fflush(stdin);
        size_t edges_read = 0;
        BPTree<AdjacentLocation> edges;
        while (1) {
            AdjacentLocation edge;
            if (fscanf(file, "%u|%u ", &edge.parent_id, &edge.child_id) == EOF) {
                break;
            }
            edges.insert((int32_t)edge.parent_id, edge);
            edges_read++;
        }
        data.adj.edges = edges;
        printf("got %zi\n", edges_read);
        return 0;
    },
    // run_input
    [] (BPTree<Location>& locs, ModeData& data, char* input) {
        uint32_t loc_id;
        if (sscanf(input, "%i", &loc_id) != 1) {
            printf("invalid id\n");
        } else {
            printf("searching id %i: ", loc_id);
            Location loc;
            if (locs.search((int32_t)loc_id, loc)) {
                printf("found \"%s\"\n", loc.name);
            } else {
                printf("not found\n");
            }
        }
        return 0;
    },
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
