#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include "graph.hpp"
#include "user_pref.hpp"

void loadLocationsFromCSV(Graph& graph, const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line); // skip header
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string city, latStr, lonStr;
        if (std::getline(ss, city, ',') && std::getline(ss, latStr, ',') && std::getline(ss, lonStr)) {
            double lat = std::stod(latStr);
            double lon = std::stod(lonStr);
            graph.addLocation(city, lat, lon);
        }
    }
}

bool isValidLocationID(const Graph& graph, int id) {
    try {
        graph.getLocationName(id);
        return true;
    } catch (...) {
        return false;
    }
}

int main() {
    Graph transport;
    DecisionTree tree;

    loadLocationsFromCSV(transport, "indonesia_cities_30.csv");

    transport.addEdge(1, 2, 180, 50000, 150);
    transport.addEdge(2, 3, 300, 70000, 200);
    transport.addEdge(3, 4, 120, 30000, 90);
    transport.addEdge(4, 5, 90, 25000, 75);

    int choice;
    do {
        std::cout << "\n=== MENU UTAMA ===\n";
        std::cout << "1. Tambah Lokasi\n";
        std::cout << "2. Tambah Rute\n";
        std::cout << "3. Hapus Lokasi\n";
        std::cout << "4. Hapus Rute\n";
        std::cout << "5. Lihat Isi Graf\n";
        std::cout << "6. Tampilkan Decision Tree\n";
        std::cout << "7. Cari Rute Terbaik\n";
        std::cout << "0. Keluar\n";
        std::cout << "Pilih menu: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            std::string name;
            double lat, lon;
            std::cout << "Nama lokasi: "; std::getline(std::cin, name);
            std::cout << "Latitude: "; std::cin >> lat;
            std::cout << "Longitude: "; std::cin >> lon;
            transport.addLocation(name, lat, lon);
        } else if (choice == 2) {
            std::cout << "\nDaftar lokasi tersedia:\n";
            transport.showLocationList();
            int from, to;
            double time, cost, dist;
            std::cout << "ID lokasi asal: "; std::cin >> from;
            std::cout << "ID lokasi tujuan: "; std::cin >> to;
            if (!isValidLocationID(transport, from) || !isValidLocationID(transport, to)) {
                std::cout << "ID lokasi tidak valid.\n";
                continue;
            }
            std::cout << "Waktu tempuh (menit): "; std::cin >> time;
            std::cout << "Biaya (Rp): "; std::cin >> cost;
            std::cout << "Jarak (km): "; std::cin >> dist;
            transport.addEdge(from, to, time, cost, dist);
        } else if (choice == 3) {
            std::cout << "\nDaftar lokasi tersedia:\n";
            transport.showLocationList();
            int id;
            std::cout << "ID lokasi yang ingin dihapus: "; std::cin >> id;
            transport.deleteLocation(id);
        } else if (choice == 4) {
            std::cout << "\nDaftar lokasi tersedia:\n";
            transport.showLocationList();
            int from, to;
            std::cout << "ID lokasi asal: "; std::cin >> from;
            std::cout << "ID lokasi tujuan: "; std::cin >> to;
            transport.deleteEdge(from, to);
        } else if (choice == 5) {
            transport.printGraph();
        } else if (choice == 6) {
            tree.visualize();
        } else if (choice == 7) {
            std::cout << "\nDaftar lokasi tersedia:\n";
            transport.showLocationList();
            int from, to;
            std::cout << "ID lokasi asal: "; std::cin >> from;
            std::cout << "ID lokasi tujuan: "; std::cin >> to;
            if (!isValidLocationID(transport, from) || !isValidLocationID(transport, to)) {
                std::cout << "ID lokasi tidak valid.\n";
                continue;
            }
            std::cin.ignore();
            UserPreference pref = tree.run();
            try {
                transport.findShortestPath(from, to, pref);
            } catch (const std::out_of_range& e) {
                std::cerr << "Terjadi kesalahan: " << e.what() << "\nPastikan rute tersedia dan ID benar.\n";
            }
        }

    } while (choice != 0);

    std::cout << "\nTerima kasih telah menggunakan sistem.\n";
    return 0;
}