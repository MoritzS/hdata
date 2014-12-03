#include <config.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include <readline/readline.h>
#include <readline/history.h>
#include <tclap/CmdLine.h>

#include "bptree.h"
#include "locations.h"

#define MAX_LOCATIONS 4000000


char const MODE_ADJ[] = "adj";
char const MODE_NI[] = "ni";
char const MODE_DELTANI[] = "deltani";

std::vector<std::string> MODES = {MODE_ADJ, MODE_NI, MODE_DELTANI};


int main(int argc, char** argv) {
    TCLAP::CmdLine args(PACKAGE_STRING);
    TCLAP::ValueArg<std::string> locsArg(
        "l", "locations",
        "Path to locations.tbl file",
        true,
        "",
        "file path"
    );
    TCLAP::ValuesConstraint<std::string> modeConstraint(MODES);
    TCLAP::UnlabeledValueArg<std::string> modeArg(
        "mode",
        "Mode of execution",
        true,
        "",
        &modeConstraint
    );

    args.add(locsArg);
    args.add(modeArg);

    args.parse(argc, argv);

    std::string mode = modeArg.getValue();

    if (mode != MODE_ADJ) {
        fprintf(stderr, "Only \"adj\" mode is implemented!\n");
        return 1;
    }

    printf("Running in \"%s\" mode\n", mode.c_str());

    Location* locs = (Location*)calloc(MAX_LOCATIONS, sizeof(Location));
    if (locs == nullptr) {
        fprintf(stderr, "couldn't allocate memory\n");
        return 1;
    }
    char const* filename = locsArg.getValue().c_str();
    BPTree locs_tree;
    printf("reading locations... ");
    FILE* locs_file = fopen(filename, "r");
    size_t num_locs = read_locations(locs_file, MAX_LOCATIONS, locs);
    fclose(locs_file);
    if (num_locs == 0) {
        printf("error\n");
        free(locs);
        return 1;
    }
    for (size_t i=0; i<num_locs; i++) {
        locs_tree.insert((int32_t)locs[i].id, locs+i);
    }
    printf("got %zi\n", num_locs);
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
