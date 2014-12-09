#include "locations.h"

#include <iostream>
#include <stack>
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
        data.adj.edges = BPTree<AdjacentLocation>();
        BPTree<AdjacentLocation>& edges = data.adj.edges;
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
        cout << "got " << edges_read << endl;
        return 0;
    },
    // run_input
    [] (BPTree<Location>& locs, ModeData& data, std::string& input) {
        using namespace std;
        stringstream stream(input);
        string s;
        stream >> s;
        if (s == "search") {
            uint32_t loc_id;
            if (!stream_ui(stream, loc_id)) {
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
        } else if (s == "num_childs") {
            uint32_t parent_id;
            if (!stream_ui(stream, parent_id)) {
                cout << "invalid id" << endl;
                return 0;
            }
            cout << "number of childs: " << data.adj.edges.count_key(parent_id) << endl;
        } else if (s == "child_ids") {
            uint32_t parent_id;
            if (!stream_ui(stream, parent_id)) {
                cout << "invalid id" << endl;
                return 0;
            }
            cout << "child ids: " << endl;
            for (AdjacentLocation& edge : data.adj.edges.search_iter(parent_id)) {
                cout << edge.child_id << endl;
            }
        } else if (s == "is_ancestor") {
            uint32_t parent_id;
            uint32_t child_id;
            if (!(stream_ui(stream, parent_id) && stream_ui(stream, child_id))) {
                cout << "invalid ids" << endl;
                return 0;
            }
            stack<uint32_t> search_ids;
            search_ids.push(parent_id);
            while (!search_ids.empty()) {
                uint32_t search_id = search_ids.top();
                search_ids.pop();
                for (AdjacentLocation& edge : data.adj.edges.search_iter(search_id)) {
                    if (edge.child_id == child_id) {
                        goto is_ancestor;
                    }
                    search_ids.push(edge.child_id);
                }
            }
            cout << "id " << parent_id << " is NOT an ancestor of id " << child_id << endl;
            return 0;
            is_ancestor:
            cout << "id " << parent_id << " is an ancestor of id " << child_id << endl;
        } else {
            cout << "unknown command '" << s << "'" << endl;
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
