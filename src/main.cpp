#include <iostream>
#include "graph.hpp"
#include "user_pref.hpp"

int main() {
    try {
        Graph transport;
        DecisionTree tree;

        // Tambahkan lokasi
        transport.addLocation("Jakarta (JKT)", -6.17, 106.82);
        transport.addLocation("Bandung (BDO)", -6.91, 107.61);
        transport.addLocation("Surabaya (SUB)", -7.25, 112.75);

        // Tambahkan rute
        transport.addEdge(1, 2, 3.0, 150000, 150);
        transport.addEdge(1, 3, 1.5, 900000, 780);
        transport.addEdge(2, 3, 10.0, 400000, 650);

        tree.visualize();
        UserPreference pref = tree.run();

        transport.findShortestPath(1, 3, pref);

    } catch (const std::exception& e) {
        std::cerr << "Terjadi kesalahan: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
