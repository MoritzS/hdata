#ifndef _LOCATIONS_H
#define _LOCATIONS_H

#include <cstdint>
#include <fstream>

#include "adj_list.h"
#include "deltani.h"
#include "nested_intervals.h"
#include "bptree.h"

struct Location {
    uint32_t id;
    char name[128];
};

typedef Hierarchy<uint32_t, Location> LocationHierarchy;

typedef AdjacencyList<uint32_t, Location> AdjLocation;
typedef NestedIntervals<uint32_t, Location> NILocation;
typedef DeltaNI<uint32_t, Location> DeltaNILocation;

using AdjacentEdge = typename AdjLocation::AdjacentEdge;
using NIEdge = typename NILocation::NIEdge;

using LocationTree = typename LocationHierarchy::ValueTree;
using AdjacencyTree = typename AdjLocation::AdjacencyTree;
using NIEdgeTree = typename NILocation::NIEdgeTree;

size_t read_locations(std::ifstream& file, LocationTree& output);
size_t read_adj_edges(std::ifstream& file, AdjacencyTree& output);
size_t read_ni_edges(std::ifstream& file, NIEdgeTree& output);

#endif
