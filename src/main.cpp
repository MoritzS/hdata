#include <config.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <readline/readline.h>
#include <readline/history.h>
#include <tclap/CmdLine.h>

#include "locations.h"
#include "util.h"


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

    LocationHierarchy* hierarchy;
    if (strMode == MODE_STR_DELTANI) {
        NIEdgeTree edges;
        cout << "reading ni edges... ";
        cout.flush();
        cout << "got " << read_ni_edges(tree_file, edges) << endl;
        hierarchy = new DeltaNILocation(locs_tree, edges);
    } else if (strMode == MODE_STR_NI) {
        NIEdgeTree edges;
        cout << "reading ni edges... ";
        cout.flush();
        cout << "got " << read_ni_edges(tree_file, edges) << endl;
        hierarchy = new NILocation(locs_tree, edges);
    } else {
        AdjacencyTree edges;
        cout << "reading edges... ";
        cout.flush();
        cout << "got " << read_adj_edges(tree_file, edges) << endl;
        hierarchy = new AdjLocation(locs_tree, edges);
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
        }

        stringstream stream(input);
        string cmd;
        stream >> cmd;
        try {
            if (cmd == "q" || cmd == "quit") {
                break;
            } else if (cmd == "help") {
                cout << "Help:" << endl;
            } else if (cmd == "s" || cmd == "search") {
                uint32_t key = stream_ui(stream);
                Location l = hierarchy->search(key);
                cout << "location with id " << l.id << ": " << l.name << endl;
            } else if (cmd == "e" || cmd == "exists") {
                uint32_t key = stream_ui(stream);
                bool exists;
                if (stream.good()) {
                    uint32_t version = key;
                    key = stream_ui(stream);
                    exists = hierarchy->exists(key, version);
                } else {
                    exists = hierarchy->exists(key);
                }
                cout << "id " << key << " ";
                if (exists) {
                    cout << "exists";
                } else {
                    cout << "doesn't exist";
                }
                cout << endl;
            } else if (cmd == "n" || cmd == "num_childs") {
                uint32_t num_childs;
                uint32_t key = stream_ui(stream);
                if (stream.good()) {
                    uint32_t version = key;
                    key = stream_ui(stream);
                    num_childs = hierarchy->num_childs(key, version);
                } else {
                    num_childs = hierarchy->num_childs(key);
                }
                cout << "id " << key << " has " << num_childs << " childs" << endl;
            } else if (cmd == "ch" || cmd == "children") {
                uint32_t key = stream_ui(stream);
                vector<uint32_t> children;
                if (stream.good()) {
                    uint32_t version = key;
                    key = stream_ui(stream);
                    children = hierarchy->children(key, version);
                } else {
                    children = hierarchy->children(key);
                }
                cout << "children of id " << key << ":" << endl;
                for (uint32_t child : children) {
                    cout << child << endl;
                }
            } else if (cmd == "a" || cmd == "is_ancestor") {
                uint32_t parent = stream_ui(stream);
                uint32_t child = stream_ui(stream);
                bool is_ancestor;
                if (stream.good()) {
                    uint32_t version = parent;
                    parent = child;
                    child = stream_ui(stream);
                    is_ancestor = hierarchy->is_ancestor(parent, child, version);
                } else {
                    is_ancestor = hierarchy->is_ancestor(parent, child);
                }
                cout << "id " << parent << " is ";
                if (!is_ancestor) {
                    cout << "NOT ";
                }
                cout << "ancestor of id " << child << endl;
            } else if (cmd == "i" || cmd == "insert") {
                uint32_t new_id = stream_ui(stream);
                string new_name;
                stream >> new_name;
                if (new_name.length() > 127) {
                    throw logic_error("name can't have more than 127 characters");
                }
                uint32_t parent = stream_ui(stream);
                Location new_loc;
                new_loc.id = new_id;
                new_name.copy(new_loc.name, 127);
                new_loc.name[new_name.length()] = 0;
                hierarchy->insert(parent, new_id, new_loc);
                cout << "inserted " << new_name << " with id " << new_id << " with parent " << parent << endl;
            } else if (cmd == "r" || cmd == "remove") {
                uint32_t id = stream_ui(stream);
                hierarchy->remove(id);
                cout << "removed id " << id << endl;
            } else if (cmd == "c" || cmd == "commit") {
                size_t new_version = hierarchy->commit();
                cout << "commited changes, new version is " << new_version << endl;
            } else {
                cout << "unknown command '" << cmd << "' try 'help'" << endl;
            }
        } catch (hierarchy_error& e) {
            cout << e.what() << endl;
        } catch (logic_error& e) {
            cout << "invalid argument: " << e.what() << endl;
        }
        add_history(line);
    }
    delete hierarchy;
    return 0;
}
