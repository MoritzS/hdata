#include "locations.h"

#include <fstream>
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

size_t read_adj_edges(std::ifstream& file, AdjacencyTree& output) {
    using namespace std;
    size_t edges_read = 0;
    while (1) {
        string line;
        getline(file, line);
        if (file.eof()) {
            break;
        }
        AdjacentEdge edge;
        stringstream line_stream(line);
        try {
            string s;
            getline(line_stream, s, '|');
            edge.parent = stou_safe(s);
            getline(line_stream, s, '|');
            edge.child = stou_safe(s);
        } catch (logic_error& e) {
            continue;
        }
        output.insert(edge.parent, edge);
        edges_read++;
    }
    return edges_read;
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
            edge.key = stou_safe(s);
            getline(line_stream, s, '|');
            edge.lower = stou_safe(s);
            getline(line_stream, s, '|');
            edge.upper = stou_safe(s);
        } catch (logic_error& e) {
            continue;
        }
        output.insert(edge.key, edge);
        edges_read++;
    }
    return edges_read;
}
