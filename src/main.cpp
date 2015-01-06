#include <config.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <readline/readline.h>
#include <readline/history.h>
#include <tclap/CmdLine.h>

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
    using namespace std;

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

    string strMode = modeArg.getValue();
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

    if (!(mode == MODE_ADJ || mode == MODE_NI)) {
        cerr << "Only \"adj\" and \"ni\" modes are implemented!" << endl;
        return 1;
    }

    cout << "Running in \"" << strMode << "\" mode" << endl;

    ifstream locs_file(locsArg.getValue());
    if (locs_file.fail()) {
        cerr << "couldn't read locations file" << endl;
        return 1;
    }
    LocationTree locs_tree;
    size_t num_locs;
    cout << "reading locations... ";
    cout.flush();
    num_locs = read_locations(locs_file, locs_tree);
    locs_file.close();
    if (num_locs == 0) {
        cerr << "error" << endl;
        return 1;
    }
    cout << "got " << num_locs << endl;

    ifstream tree_file(treeArg.getValue());
    if (tree_file.fail()) {
        cerr << "couldn't read tree data file" << endl;
        return 1;
    }
    ModeData mode_data;
    if (mode_info.init_mode != nullptr) {
        int ret = mode_info.init_mode(tree_file, mode_data);
        if (ret != 0) {
            return ret;
        }
    }
    tree_file.close();

    char *line;
    while(1) {
        line = readline("> ");
        if (line == nullptr) {
            cout << endl;
            break;
        }
        string input(line);
        if (input.empty()) {
            continue;
        } else if (input == "q" || input == "quit") {
            break;
        }
        if (mode_info.run_input(locs_tree, mode_data, input) == 0) {
            add_history(line);
        }
    }
    if (mode_info.exit_mode != nullptr) {
        int ret = mode_info.exit_mode(mode_data);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}
