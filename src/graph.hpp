#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <map>
#include <vector>
#include <queue>
#include <cmath>
#include <fstream>
#include <iostream>
#include "user_pref.hpp"
#include "json.hpp"

using json = nlohmann::json;

class Location {
private:
    int id;
    std::string name;
    double lat, lon;
public:
    Location() : id(0), name(""), lat(0), lon(0) {}
    Location(int i, const std::string& n, double la, double lo)
        : id(i), name(n), lat(la), lon(lo) {}
    int getId() const { return id; }
    std::string getName() const { return name; }
    double getLat() const { return lat; }
    double getLon() const { return lon; }
};

struct Edge {
    int to;
    double time, cost, distance;
    Edge(int t, double ti, double co, double di)
        : to(t), time(ti), cost(co), distance(di) {}
};

class Graph {
private:
    std::map<int, Location> locations;
    std::map<int, std::vector<Edge>> adj;
    int nextId = 1;

    double heuristic(int a, int b) const {
        const Location& la = locations.at(a);
        const Location& lb = locations.at(b);
        double dx = la.getLat() - lb.getLat();
        double dy = la.getLon() - lb.getLon();
        return std::sqrt(dx * dx + dy * dy);
    }

    void exportToJson(const std::vector<int>& path) const {
        json j = json::array();
        for (int id : path) {
            const Location& loc = locations.at(id);
            j.push_back({ {"name", loc.getName()}, {"lat", loc.getLat()}, {"lng", loc.getLon()} });
        }
        std::ofstream out("route_output.json");
        out << j.dump(4);
    }

public:
    void addLocation(const std::string& name, double lat, double lon) {
        locations[nextId] = Location(nextId, name, lat, lon);
        ++nextId;
    }

    void addEdge(int from, int to, double time, double cost, double distance) {
        adj[from].emplace_back(to, time, cost, distance);
    }

    void findShortestPath(int start, int goal, const UserPreference& pref) {
        std::map<int, double> gScore, fScore;
        std::map<int, int> cameFrom;
        auto cmp = [&](int a, int b) { return fScore[a] > fScore[b]; };
        std::priority_queue<int, std::vector<int>, decltype(cmp)> openSet(cmp);

        for (const auto& [id, _] : locations) {
            gScore[id] = fScore[id] = std::numeric_limits<double>::infinity();
        }
        gScore[start] = 0;
        fScore[start] = heuristic(start, goal);
        openSet.push(start);

        while (!openSet.empty()) {
            int current = openSet.top(); openSet.pop();
            if (current == goal) break;

            for (const Edge& e : adj[current]) {
                double weight = e.time * pref.timeWeight + e.cost * pref.costWeight + e.distance * pref.distanceWeight;
                double tentative_g = gScore[current] + weight;
                if (tentative_g < gScore[e.to]) {
                    cameFrom[e.to] = current;
                    gScore[e.to] = tentative_g;
                    fScore[e.to] = tentative_g + heuristic(e.to, goal);
                    openSet.push(e.to);
                }
            }
        }

        std::vector<int> path;
        for (int at = goal; at != start; at = cameFrom[at]) {
            path.push_back(at);
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());

        std::cout << "\n>>> Rute terbaik:\n";
        for (int id : path) std::cout << " - " << locations[id].getName() << "\n";

        exportToJson(path);
        std::cout << "\nFile 'route_output.json' berhasil dibuat. Buka map.html untuk melihat.\n";
    }
};

#endif