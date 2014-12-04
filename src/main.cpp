#include <config.h>

#include <cerrno>
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


enum Mode {
    MODE_ADJ = 0,
    MODE_NI = 1,
    MODE_DELTANI = 2
};

char const MODE_STR_ADJ[] = "adj";
char const MODE_STR_NI[] = "ni";
char const MODE_STR_DELTANI[] = "deltani";

std::vector<std::string> MODES = {MODE_STR_ADJ, MODE_STR_NI, MODE_STR_DELTANI};

ModeInfo const MODES_INFO[] = {adjModeInfo, niModeInfo, deltaniModeInfo};


int main(int argc, char** argv) {
    TCLAP::CmdLine args(PACKAGE_STRING);
    TCLAP::ValuesConstraint<std::string> modeConstraint(MODES);
    TCLAP::UnlabeledValueArg<std::string> modeArg(
        "mode",
        "Mode of execution",
        true,
        "",
        &modeConstraint
    );
    TCLAP::UnlabeledValueArg<std::string> locsArg(
        "locations",
        "Path to locations.tbl file",
        true,
        "",
        "file path"
    );
    TCLAP::UnlabeledValueArg<std::string> treeArg(
        "tree",
        "Path to mode specific tree data",
        true,
        "",
        "file path"
    );

    args.add(modeArg);
    args.add(locsArg);
    args.add(treeArg);

    args.parse(argc, argv);

    std::string strMode = modeArg.getValue();
    Mode mode;
    if (strMode == MODE_STR_DELTANI) {
        mode = MODE_DELTANI;
    } else if (strMode == MODE_STR_NI) {
        mode = MODE_NI;
    } else {
        mode = MODE_ADJ;
    }

    if (mode != MODE_ADJ) {
        fprintf(stderr, "Only \"adj\" mode is implemented!\n");
        return 1;
    }

    printf("Running in \"%s\" mode\n", strMode.c_str());

    char const* filename = locsArg.getValue().c_str();
    BPTree<Location> locs_tree;
    FILE* locs_file = fopen(filename, "r");
    size_t num_locs;
    if (locs_file == nullptr) {
        perror("couldn't read locations file");
        return 1;
    }
    printf("reading locations... ");
    num_locs = read_locations(locs_file, MAX_LOCATIONS, locs_tree);
    fclose(locs_file);
    if (num_locs == 0) {
        printf("error\n");
        return 1;
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
            Location loc;
            if (locs_tree.search((int32_t)loc_id, loc)) {
                printf("found \"%s\"\n", loc.name);
            } else {
                printf("not found\n");
            }
        }
        add_history(line);
    }
    return 0;
}
