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


enum Mode {
    MODE_ADJ = 0,
    MODE_NI = 1,
    MODE_DELTANI = 2
};

char const MODE_STR_ADJ[] = "adj";
char const MODE_STR_NI[] = "ni";
char const MODE_STR_DELTANI[] = "deltani";

std::vector<std::string> MODES = {MODE_STR_ADJ, MODE_STR_NI, MODE_STR_DELTANI};


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
    ModeInfo mode_info;
    if (strMode == MODE_STR_DELTANI) {
        mode = MODE_DELTANI;
        mode_info = deltaniModeInfo;
    } else if (strMode == MODE_STR_NI) {
        mode = MODE_NI;
        mode_info = niModeInfo;
    } else {
        mode = MODE_ADJ;
        mode_info = adjModeInfo;
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
    fflush(stdout);
    num_locs = read_locations(locs_file, locs_tree);
    fclose(locs_file);
    if (num_locs == 0) {
        printf("error\n");
        return 1;
    }
    printf("got %zi\n", num_locs);

    filename = treeArg.getValue().c_str();
    FILE* tree_file = fopen(filename, "r");
    if (tree_file == nullptr) {
        perror("couldn't read tree data file");
        return 1;
    }
    ModeData mode_data;
    if (mode_info.init_mode != nullptr) {
        int ret = mode_info.init_mode(tree_file, mode_data);
        if (ret != 0) {
            return ret;
        }
    }

    char *line;
    while(1) {
        line = readline("> ");
        if (line == nullptr) {
            printf("\n");
            break;
        }
        if (line[0] == 0) {
            continue;
        }
        if (mode_info.run_input(locs_tree, mode_data, line) == 0) {
            add_history(line);
        }
    }
    if (mode_info.exit_mode != nullptr) {
        int ret = mode_info.exit_mode(locs_tree, mode_data);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}
