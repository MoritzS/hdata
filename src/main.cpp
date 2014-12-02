#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <readline/readline.h>
#include <readline/history.h>

#include "bptree.h"
#include "locations.h"

#define MAX_LOCATIONS 4000000

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s filename\n", argv[0]);
        return 1;
    }
    Location* locs = (Location*)calloc(MAX_LOCATIONS, sizeof(Location));
    if (locs == nullptr) {
        printf("couldn't allocate memory\n");
        return 1;
    }
    char* filename = argv[1];
    BPTree locs_tree;
    FILE* locs_file = fopen(filename, "r");
    size_t num_locs = read_locations(locs_file, MAX_LOCATIONS, locs);
    if (num_locs == 0) {
        printf("couldn't read locations\n");
        free(locs);
        return 1;
    }
    for (size_t i=0; i<num_locs; i++) {
        locs_tree.insert((int32_t)locs[i].id, locs+i);
    }
    printf("read %zi locations\n", num_locs);
    printf("depth: %zi\n", locs_tree.depth());
    char *line;
    while(1) {
        line = readline("> ");
        if (line == nullptr) {
            printf("\n");
            break;
        }
        uint32_t loc_id;
        if (sscanf(line, "%i", &loc_id) != 1) {
            printf("invalid id\n");
        } else {
            printf("searching id %i: ", loc_id);
            Location* loc;
            if (locs_tree.search((int32_t)loc_id, (void**)&loc)) {
                printf("found \"%s\"\n", loc->name);
            } else {
                printf("not found\n");
            }
        }
        add_history(line);
    }
    free(locs);
    return 0;
}
