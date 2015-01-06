#include "locations.h"

#include <iostream>
#include <map>
#include <stack>
#include <sstream>
#include <stdexcept>

#include "util.h"


size_t read_locations(std::ifstream& file, LocationTree& tree) {
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
            tree.insert(loc.id, loc);
            locations_read++;
        }
    }
    return locations_read;
}

ModeInfo adjModeInfo = {
    // init_mode
    [] (std::ifstream& file, ModeData& data) {
        using namespace std;
        data.adj = new AdjModeData();
        cout << "reading edges... ";
        cout.flush();
        size_t edges_read = 0;
        AdjLocTree& edges = data.adj->edges;
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
            edges.insert(edge.parent_id, edge);
            edges_read++;
        }
        cout << "got " << edges_read << endl;
        return 0;
    },
    // run_input
    [] (LocationTree& locs, ModeData& data, std::string& input) {
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
            if (locs.search(loc_id, loc)) {
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
            cout << "number of childs: " << data.adj->edges.count_key(parent_id) << endl;
        } else if (s == "child_ids") {
            uint32_t parent_id;
            if (!stream_ui(stream, parent_id)) {
                cout << "invalid id" << endl;
                return 0;
            }
            cout << "child ids: " << endl;
            for (AdjacentLocation& edge : data.adj->edges.search_iter(parent_id)) {
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
                for (AdjacentLocation& edge : data.adj->edges.search_iter(search_id)) {
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
    // exit_mode
    [] (ModeData& data) {
        delete data.adj;
        return 0;
    }
};

ModeInfo niModeInfo = {
    // init_mode
    [] (std::ifstream& file, ModeData& data) {
        using namespace std;
        data.ni = new NiModeData();
        cout << "reading ni edges... ";
        cout.flush();
        size_t edges_read = 0;
        NIEdgeTree& edges = data.ni->edges;
        while (1) {
            string line;
            getline(file, line);
            if (file.eof()) {
                break;
            }
            NIEdge edge;
            stringstream line_stream(line);
            try {
                string s;
                getline(line_stream, s, '|');
                edge.loc_id = stou_safe(s);
                getline(line_stream, s, '|');
                edge.lower = stou_safe(s);
                getline(line_stream, s, '|');
                edge.upper = stou_safe(s);
            } catch (logic_error& e) {
                continue;
            }
            edges.insert(edge.loc_id, edge);
            edges_read++;
        }
        cout << "got " << edges_read << endl;
        return 0;
    },
    // run_input
    [] (LocationTree& locs, ModeData& data, std::string& input) {
        using namespace std;
        stringstream stream(input);
        string s;
        stream >> s;
        if (s == "convert_adj") {
            uint32_t root_id;
            if (!(stream_ui(stream, root_id))) {
                cout << "invalid root id" << endl;
                return 0;
            }
            string filename_from;
            string filename_to;
            stream >> filename_from >> filename_to;
            ifstream from_file(filename_from);
            if (from_file.fail()) {
                cout << "couldn't read input file" << endl;
                return 0;
            }
            ofstream to_file(filename_to);
            if (to_file.fail()) {
                cout << "couldn't read output file" << endl;
                return 0;
            }
            ModeData adj_data;
            adjModeInfo.init_mode(from_file, adj_data);
            uint32_t dfs_num = 1;
            stack<uint32_t> search_ids;
            map<uint32_t, uint32_t> start_nums;
            search_ids.push(root_id);
            cout << "generating ni data... ";
            cout.flush();
            while (!search_ids.empty()) {
                uint32_t search_id = search_ids.top();
                map<uint32_t, uint32_t>::iterator it = start_nums.find(search_id);
                if (it != start_nums.end()) {
                    to_file << search_id << '|' << it->second << '|' << dfs_num << endl;
                    dfs_num++;
                    search_ids.pop();
                    start_nums.erase(it);
                    continue;
                } else {
                    start_nums.insert(pair<uint32_t, uint32_t>(search_id, dfs_num));
                    dfs_num++;
                }
                for (AdjacentLocation& edge : adj_data.adj->edges.search_iter(search_id)) {
                    search_ids.push(edge.child_id);
                }
            }
            adjModeInfo.exit_mode(adj_data);
            cout << "done" << endl;
        } else if (s == "is_ancestor") {
            uint32_t parent_id;
            uint32_t child_id;
            if (!(stream_ui(stream, parent_id) && stream_ui(stream, child_id))) {
                cout << "invalid ids" << endl;
                return 0;
            }
            NIEdge parent_ni;
            NIEdge child_ni;
            if (data.ni->edges.search(parent_id, parent_ni) &&
                data.ni->edges.search(child_id, child_ni)) {
                if (parent_ni.lower < child_ni.lower && parent_ni.upper > child_ni.upper) {
                    cout << "id " << parent_id << " is an ancestor of id " << child_id << endl;
                    return 0;
                }
            }
            cout << "id " << parent_id << " is NOT an ancestor of id " << child_id << endl;
        } else {
            cout << "unknown command '" << s << "'" << endl;
        }
        return 0;
    },
    // exit_mode
    [] (ModeData& data) {
        delete data.ni;
        return 0;
    }
};

ModeInfo deltaniModeInfo = {
    nullptr,
    nullptr,
    nullptr
};
