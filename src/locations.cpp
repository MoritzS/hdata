#include "locations.h"

#include <iostream>
#include <map>
#include <stack>
#include <sstream>
#include <utility>

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

size_t read_ni_edges(std::ifstream& file, NIEdgeTree& output) {
    using namespace std;
    size_t edges_read = 0;
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
        output.insert(edge.loc_id, edge);
        edges_read++;
    }
    return edges_read;
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
        cout << "got " << read_ni_edges(file, data.ni->edges) << endl;
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


DeltaFunction::DeltaFunction()
:ranges(), ranges_inv(), max(0) {
}

bool DeltaFunction::empty() const {
    return ranges.empty();
}

void DeltaFunction::add_range(DeltaRange const& range) {
    ranges.insert(range.from, range);
    ranges_inv.insert(range.to, range);
}

uint32_t DeltaFunction::evaluate(uint32_t const value) const {
    if (!ranges.empty()) {
        DeltaRange const& range = *ranges.search_range(value).begin();
        return value + range.to - range.from;
    } else {
        return value;
    }
}

uint32_t DeltaFunction::evaluate_inv(uint32_t const value) const {
    if (!ranges_inv.empty()) {
        DeltaRange const& range = *ranges_inv.search_range(value).begin();
        return value - range.to + range.from;
    } else {
        return value;
    }
}

NIEdge DeltaFunction::apply(NIEdge const& edge) const {
    NIEdge new_edge;
    new_edge.loc_id = edge.loc_id;
    new_edge.lower = evaluate(edge.lower);
    new_edge.upper = evaluate(edge.upper);
    return new_edge;
}

DeltaFunction DeltaFunction::merge(DeltaFunction const& delta) const {
    if (delta.ranges.empty()) {
        return *this;
    } else if (ranges.empty()) {
        return delta;
    }
    DeltaFunction new_delta;
    new_delta.max = delta.max;
    for (DeltaRange range : ranges) {
        new_delta.add_range({range.from, delta.evaluate(range.to)});
    }
    for (DeltaRange range : delta.ranges) {
        uint32_t from = evaluate_inv(range.from);
        if (new_delta.ranges.count_key(from) == 0) {
            new_delta.add_range({from, range.to});
        }
    }
    return new_delta;
}

DeltaVersions::DeltaVersions()
: init_max(0), max_edge(0), edges(), deltas(), wip_delta() {
}

DeltaVersions::DeltaVersions(NIEdgeTree& edges)
: max_edge(0), edges(std::move(edges)), deltas(), wip_delta() {
    // search root (edge with e.lower == 1)
    for (NIEdge& e : this->edges) {
        if (e.lower == 1) {
            init_max = e.upper + 1;
        }
        if (e.upper > max_edge) {
            max_edge = e.upper;
        }
    }
}

DeltaVersions::DeltaVersions(NIEdgeTree& edges, uint32_t const max, uint32_t const max_edge)
: init_max(max), max_edge(max_edge), edges(std::move(edges)), deltas(), wip_delta() {
}

size_t DeltaVersions::max_version() const {
    if (deltas.empty()) {
        return 0;
    } else {
        return deltas[0].size();
    }
}

NIEdge DeltaVersions::get_edge(NIEdge const& edge, size_t const version, bool const use_wip) const {
    size_t v;
    if (version > max_version()) {
        v = max_version();
    } else {
        v = version;
    }
    if (v == 0) {
        if (wip_delta.empty()) {
            return edge;
        } else {
            return wip_delta.apply(edge);
        }
    }

    // power = floor(log2(version));
    size_t power = sizeof(size_t) * 8 - 1;
    while (v >> power == 0) {
        power--;
    }

    NIEdge new_edge = edge;
    size_t current_version = 0;
    while (current_version < v) {
        size_t step = UINTMAX_C(1) << power;
        new_edge = deltas[power][current_version / step].apply(new_edge);
        current_version += step;
        if (power > 0) {
            power--;
            while (((v >> power) & 1) == 0) {
                power--;
            }
        }
    };

    if (use_wip) {
        new_edge = wip_delta.apply(new_edge);
    }

    return new_edge;
}

NIEdge DeltaVersions::get_edge(NIEdge const& edge) const {
    return get_edge(edge, max_version(), true);
}

NIEdge DeltaVersions::get_edge(NIEdge const& edge, size_t const version) const {
    return get_edge(edge, version, false);
}

size_t DeltaVersions::insert_delta(DeltaFunction const& delta) {
    if (deltas.empty()) {
        deltas.emplace_back();
        deltas[0].push_back(delta);
        return 1;
    } else {
        deltas[0].push_back(delta);
        size_t size = deltas[0].size();
        for (size_t level = 0; size % 2 == 0; level++) {
            if (level + 1 >= deltas.size()) {
                deltas.emplace_back();
            }
            DeltaFunction merged = deltas[level][size-2].merge(deltas[level][size-1]);
            deltas[level + 1].push_back(merged);
            size = deltas[level + 1].size();
        }
        return deltas[0].size();
    }
}

bool DeltaVersions::exists(uint32_t const id, size_t const version, bool const use_wip) const {
    NIEdge edge;
    if (!edges.search(id, edge)) {
        return false;
    }
    if (version == 0) {
        if (use_wip && !wip_delta.empty()) {
            return wip_delta.evaluate(edge.lower) < wip_delta.max;
        } else {
            return edge.lower < init_max;
        }
    } else {
        edge = get_edge(edge, version, use_wip);
        if (use_wip && !wip_delta.empty()) {
            return edge.lower < wip_delta.max;
        } else {
            return edge.lower < deltas[0][version - 1].max;
        }
    }
}

bool DeltaVersions::exists(uint32_t const id) const {
    return exists(id, max_version(), true);
}

bool DeltaVersions::exists(uint32_t const id, size_t const version) const {
    return exists(id, version, false);
}

bool DeltaVersions::is_ancestor(uint32_t const parent_id, uint32_t const child_id, size_t const version, bool const use_wip) const {
    NIEdge parent_edge;
    NIEdge child_edge;
    if (!edges.search(parent_id, parent_edge)) {
        throw deltani_invalid_id(parent_id);
    }
    if (!edges.search(child_id, child_edge)) {
        throw deltani_invalid_id(child_id);
    }
    parent_edge = get_edge(parent_edge, version, use_wip);
    child_edge = get_edge(child_edge, version, use_wip);
    return parent_edge.lower < child_edge.lower && parent_edge.upper > child_edge.upper;
}

bool DeltaVersions::is_ancestor(uint32_t const parent_id, uint32_t const child_id) const {
    return is_ancestor(parent_id, child_id, max_version(), true);
}

bool DeltaVersions::is_ancestor(uint32_t const parent_id, uint32_t const child_id, size_t const version) const {
    return is_ancestor(parent_id, child_id, version, false);
}

void DeltaVersions::insert(uint32_t const id, uint32_t const parent_id) {
    NIEdge parent_edge;
    if (!edges.search(parent_id, parent_edge)) {
        throw deltani_invalid_id(parent_id);
    }
    if (!exists(parent_id)) {
        throw deltani_id_removed(parent_id);
    }
    parent_edge = get_edge(parent_edge);

    if (exists(id)) {
        throw deltani_id_exists(id);
    }

    NIEdge inserting_edge;
    if (edges.search(id, inserting_edge)) {
        inserting_edge = get_edge(inserting_edge);
    } else {
        inserting_edge.loc_id = id;
        inserting_edge.lower = max_edge + 1;
        inserting_edge.upper = max_edge + 2;
        edges.insert(id, inserting_edge);
        max_edge += 2;
    }
    DeltaFunction delta;
    delta.add_range({1, 1});
    delta.add_range({parent_edge.upper, parent_edge.upper + 2});
    delta.add_range({inserting_edge.lower, parent_edge.upper});
    delta.add_range({inserting_edge.upper + 1, inserting_edge.upper + 1});
    if (!wip_delta.empty()) {
        delta.max = wip_delta.max + 2;
    } else if (deltas.empty()) {
        delta.max = init_max + 2;
    } else {
        delta.max = deltas[0][deltas[0].size() - 1].max + 2;
    }

    wip_delta = wip_delta.merge(delta);
}

void DeltaVersions::remove(uint32_t const id) {
    NIEdge edge;
    if (!edges.search(id, edge)) {
        throw deltani_invalid_id(id);
    }
    if (!exists(id)) {
        throw deltani_id_removed(id);
    }
    edge = get_edge(edge);

    if (edge.upper - edge.lower > 1) {
        throw deltani_id_has_children(id);
    }

    DeltaFunction delta;
    delta.add_range({1, 1});
    if (edge.lower == 1) {
        delta.max = 1;
    } else {
        if (!wip_delta.empty()) {
            delta.max = wip_delta.max - 2;
        } else if (deltas.empty()) {
            delta.max = init_max - 2;
        } else {
            delta.max = deltas[0][deltas[0].size() - 1].max - 2;
        }
        delta.add_range({edge.lower, delta.max});
        delta.add_range({edge.upper + 1, edge.lower});
        delta.add_range({delta.max + 2, delta.max + 2});
    }

    wip_delta = wip_delta.merge(delta);
}

uint32_t DeltaVersions::save() {
    if (wip_delta.empty()) {
        return max_version();
    } else {
        uint32_t new_version = insert_delta(wip_delta);
        wip_delta = DeltaFunction();
        return new_version;
    }
}

ModeInfo deltaniModeInfo = {
    // init_mode
    [] (std::ifstream& file, ModeData& data) {
        using namespace std;
        data.deltani = new DeltaniModeData();
        cout << "reading ni edges... ";
        cout.flush();
        NIEdgeTree edges;
        cout << "got " << read_ni_edges(file, edges) << endl;
        cout << "generating initial version... ";
        cout.flush();
        data.deltani->versions = DeltaVersions(edges);
        cout << "done" << endl;
        return 0;
    },
    // run_input
    [] (LocationTree& locs, ModeData& data, std::string& input) {
        using namespace std;
        stringstream stream(input);
        string s;
        stream >> s;
        if (s == "is_ancestor") {
            uint32_t version;
            uint32_t parent_id;
            uint32_t child_id;
            if (!(stream_ui(stream, version))) {
                cout << "invalid version" << endl;
                return 0;
            }
            if (!(stream_ui(stream, parent_id) && stream_ui(stream, child_id))) {
                cout << "invalid ids" << endl;
                return 0;
            }
            try {
                if (data.deltani->versions.is_ancestor(parent_id, child_id, version)) {
                    cout << "id " << parent_id << " is an ancestor of id " << child_id << endl;
                } else {
                    cout << "id " << parent_id << " is NOT an ancestor of id " << child_id << endl;
                }
            } catch (deltani_invalid_id& e) {
                cout << "id " << e.id << " not found" << endl;
            }
        } else if (s == "insert") {
            uint32_t parent_id;
            uint32_t new_id;
            if (!(stream_ui(stream, parent_id) && stream_ui(stream, new_id))) {
                cout << "invalid ids" << endl;
                return 0;
            }
            try {
                data.deltani->versions.insert(new_id, parent_id);
                cout << "inserted" << endl;
            } catch (deltani_id_exists& e) {
                cout << "id " << e.id << " already exists" << endl;
            } catch (deltani_id_removed& e) {
                cout << "parent id " << e.id << " has already been removed" << endl;
            } catch (deltani_invalid_id& e) {
                cout << "parent id " << e.id << " invalid" << endl;
            }
        } else if (s == "save") {
            uint32_t version = data.deltani->versions.save();
            cout << "saved, newest version is " << version << endl;
        } else {
            cout << "unknown command '" << s << "'" << endl;
        }
        return 0;
    },
    // exit_mode
    [] (ModeData& data) {
        delete data.deltani;
        return 0;
    }
};
