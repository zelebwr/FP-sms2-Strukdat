#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include "user_pref.hpp"

struct Location {
    int id;
    std::string name;
    double lat;
    double lon;

    Location(int i, const std::string& n, double la, double lo)
        : id(i), name(n), lat(la), lon(lo) {}

    Location() = default;
};

struct Edge {
    int to;
    double time;
    double cost;
    double distance;

    Edge(int t, double ti, double co, double di)
        : to(t), time(ti), cost(co), distance(di) {}
};

class Graph {
private:
    std::unordered_map<int, Location> locations;
    std::unordered_map<int, std::vector<Edge>> adj;
    int nextId = 1;

    double heuristic(int a, int b) const {
        const Location& locA = locations.at(a);
        const Location& locB = locations.at(b);
        double dx = locA.lat - locB.lat;
        double dy = locA.lon - locB.lon;
        return std::sqrt(dx * dx + dy * dy);
    }

public:
    void addLocation(const std::string& name, double lat, double lon) {
        locations[nextId] = Location(nextId, name, lat, lon);
        nextId++;
    }

    void deleteLocation(int id) {
        locations.erase(id);
        adj.erase(id);
        for (auto& [from, edges] : adj) {
            edges.erase(std::remove_if(edges.begin(), edges.end(), [id](const Edge& e) {
                return e.to == id;
            }), edges.end());
        }
    }

    void addEdge(int from, int to, double time, double cost, double distance) {
        adj[from].emplace_back(to, time, cost, distance);
    }

    void deleteEdge(int from, int to) {
        if (adj.count(from)) {
            auto& edges = adj[from];
            edges.erase(std::remove_if(edges.begin(), edges.end(), [to](const Edge& e) {
                return e.to == to;
            }), edges.end());
        }
    }

    void showLocationList() const {
        std::cout << "\n=== ID Lokasi ===\n";
        for (const auto& [id, loc] : locations) {
            std::cout << id << ". " << loc.name << "\n";
        }
    }

    void printGraph() const {
        for (const auto& [from, edges] : adj) {
            std::cout << "\nDari " << locations.at(from).name << " (" << from << "):\n";
            for (const auto& e : edges) {
                const auto& dest = locations.at(e.to);
                std::cout << "  -> " << dest.name << " (" << e.to << ")"
                          << ", Waktu: " << e.time << " menit, Biaya: Rp" << e.cost
                          << ", Jarak: " << e.distance << " km\n";
            }
        }
    }

    std::string getLocationName(int id) const {
        if (locations.count(id)) return locations.at(id).name;
        throw std::out_of_range("ID lokasi tidak ditemukan.");
    }

    void findShortestPath(int start, int goal, const UserPreference& pref) {
        std::unordered_map<int, double> gScore;
        std::unordered_map<int, double> fScore;
        std::unordered_map<int, int> cameFrom;
        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> openSet;

        for (const auto& [id, _] : locations) {
            gScore[id] = std::numeric_limits<double>::infinity();
            fScore[id] = std::numeric_limits<double>::infinity();
        }

        gScore[start] = 0;
        fScore[start] = heuristic(start, goal);
        openSet.emplace(fScore[start], start);

        while (!openSet.empty()) {
            int current = openSet.top().second;
            openSet.pop();

            if (current == goal) {
                std::vector<int> path;
                for (int node = goal; node != start; node = cameFrom[node])
                    path.push_back(node);
                path.push_back(start);
                std::reverse(path.begin(), path.end());

                double totalTime = 0, totalCost = 0, totalDist = 0;
                std::cout << "\nRute terbaik:\n";
                for (size_t i = 0; i < path.size(); ++i) {
                    std::cout << locations.at(path[i]).name;
                    if (i + 1 < path.size()) {
                        for (const auto& e : adj[path[i]]) {
                            if (e.to == path[i + 1]) {
                                totalTime += e.time;
                                totalCost += e.cost;
                                totalDist += e.distance;
                                break;
                            }
                        }
                        std::cout << " -> ";
                    }
                }
                std::cout << "\nTotal waktu: " << totalTime << " menit\n";
                std::cout << "Total biaya: Rp" << totalCost << "\n";
                std::cout << "Total jarak: " << totalDist << " km\n";
                std::cout << "Jumlah transit: " << path.size() - 2 << "\n";
                return;
            }

            for (const auto& e : adj[current]) {
                double weight = e.time * pref.timeWeight + e.cost * pref.costWeight + e.distance * pref.distanceWeight;
                double tentative_g = gScore[current] + weight;
                if (tentative_g < gScore[e.to]) {
                    cameFrom[e.to] = current;
                    gScore[e.to] = tentative_g;
                    fScore[e.to] = tentative_g + heuristic(e.to, goal);
                    openSet.emplace(fScore[e.to], e.to);
                }
            }
        }

        std::cout << "\nTidak ditemukan jalur dari " << locations.at(start).name << " ke " << locations.at(goal).name << ".\n";
    }
};
