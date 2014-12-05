#include "locations.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "util.h"


size_t read_locations(std::ifstream& file, BPTree<Location>& tree) {
    using namespace std;
    size_t locations_read = 0;
    while(1) {
        string line;
        getline(file, line);
        if (file.eof()) {
            break;
        }
        Location loc;
        stringstream line_stream(line);
        string s;
        getline(line_stream, s, '|');
        try {
            loc.id = stou_safe(s);
        } catch (logic_error& e) {
            continue;
        }
        getline(line_stream, s);
        if (!s.empty()) {
            size_t length = s.copy(loc.name, 127);
            loc.name[length] = 0;
            tree.insert((int32_t)loc.id, loc);
            locations_read++;
        }
    }
    return locations_read;
}

ModeInfo adjModeInfo = {
    // init_mode
    [] (std::ifstream& file, ModeData& data) {
        using namespace std;
        cout << "reading edges... ";
        cout.flush();
        size_t edges_read = 0;
        BPTree<AdjacentLocation> edges;
        while (1) {
            string line;
            getline(file, line);
            if (file.eof()) {
                break;
            }
            AdjacentLocation edge;
            stringstream line_stream(line);
            try {
                string s;
                getline(line_stream, s, '|');
                edge.parent_id = stou_safe(s);
                getline(line_stream, s, '|');
                edge.child_id = stou_safe(s);
            } catch (logic_error& e) {
                continue;
            }
            edges.insert((int32_t)edge.parent_id, edge);
            edges_read++;
        }
        data.adj.edges = edges;
        cout << "got " << edges_read << endl;
        return 0;
    },
    // run_input
    [] (BPTree<Location>& locs, ModeData& data, std::string& input) {
        using namespace std;
        uint32_t loc_id;
        try {
            loc_id = stou_safe(input);
        } catch (logic_error& e) {
            cout << "invalid id" << endl;
            return 0;
        }
        cout << "searching id " << loc_id << ": ";
        cout.flush();
        Location loc;
        if (locs.search((int32_t)loc_id, loc)) {
            cout << "found \"" << loc.name << '"' << endl;
        } else {
            cout << "not found" << endl;
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
