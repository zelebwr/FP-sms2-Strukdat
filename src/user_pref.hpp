#ifndef USERPREFERENCE_HPP
#define USERPREFERENCE_HPP

#include <iostream>
#include <string>
#include <limits>

struct UserPreference {
    double weight_time;
    double weight_cost;
    double weight_distance;

    UserPreference(double t = 0.0, double c = 0.0, double d = 0.0)
        : weight_time(t), weight_cost(c), weight_distance(d) {}

    void normalize() {
        double total = weight_time + weight_cost + weight_distance;
        if (total > 0) {
            weight_time     /= total;
            weight_cost     /= total;
            weight_distance /= total;
        }
    }

    void display() const {
        std::cout << "Preferensi Pengguna:" << std::endl;
        std::cout << "  - Waktu    : " << weight_time << std::endl;
        std::cout << "  - Biaya    : " << weight_cost << std::endl;
        std::cout << "  - Jarak    : " << weight_distance << std::endl;
    }
};

struct Edge {
    double time;
    double cost;
    double distance;
};

class PreferenceManager {
public:
    static UserPreference inputPreference() {
        int choice;
        UserPreference pref;

        std::cout << "\n=== Pilih Preferensi Rute ===\n";
        std::cout << "1. Tercepat (Prioritas Waktu)\n";
        std::cout << "2. Termurah (Prioritas Biaya)\n";
        std::cout << "3. Terdekat (Prioritas Jarak)\n";
        std::cout << "4. Custom (Tentukan Bobot Sendiri)\n";
        std::cout << "Pilihan Anda: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                pref = UserPreference(1, 0, 0);
                break;
            case 2:
                pref = UserPreference(0, 1, 0);
                break;
            case 3:
                pref = UserPreference(0, 0, 1);
                break;
            case 4:
                std::cout << "Masukkan bobot waktu [0.0 - 1.0]: ";
                std::cin >> pref.weight_time;
                std::cout << "Masukkan bobot biaya [0.0 - 1.0]: ";
                std::cin >> pref.weight_cost;
                std::cout << "Masukkan bobot jarak [0.0 - 1.0]: ";
                std::cin >> pref.weight_distance;
                break;
            default:
                std::cout << "Pilihan tidak valid. Menggunakan default (tercepat).\n";
                pref = UserPreference(1, 0, 0);
                break;
        }

        pref.normalize();
        return pref;
    }

    static double calculateWeightedCost(const Edge& edge, const UserPreference& pref) {
        return (pref.weight_time     * edge.time) +
               (pref.weight_cost     * edge.cost) +
               (pref.weight_distance * edge.distance);
    }
};

#endif

/*
#include <iostream>
#include "UserPreference.hpp"

int main() {
    UserPreference pref = PreferenceManager::inputPreference();
    pref.display();

    // Contoh edge
    Edge edge = { 20.0, 10000.0, 8.0 }; // waktu: 20 menit, biaya: 10.000, jarak: 8 km

    double score = PreferenceManager::calculateWeightedCost(edge, pref);
    std::cout << "\nSkor total edge (berdasarkan preferensi): " << score << std::endl;

    return 0;
}
*/