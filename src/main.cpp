// =================================================================================
// HEADERS 
// =================================================================================
// Input/Output operations
#include <iostream> 
#include <fstream> // For file input/output (saving/loading data)
#include <sstream> // For string stream manipulations

// Data Structures
#include <vector> // For dynamic arrays (e.g., to store lists of routes)
#include <map> // For key-value pairs (e.g., managing nodes by ID)
#include <unordered_map> // For more efficient key-value access
#include <queue> // For implementing priority_queue (essential for Dijkstra/A*)
#include <list> // For adjacency list representation of the graph
#include <string> // For handling text data

// Algorithms and Utilities
#include <algorithm> // For sorting, searching, and other algorithms
#include <limits> // To represent infinity in pathfinding algorithms
#include <memory> // For smart pointers (e.g., unique_ptr, shared_ptr)

using namespace std;

// =================================================================================
// FORWARD DECLARATIONS
// =================================================================================

class Location;
class Route;
class Graph;

// =================================================================================
// ENCAPSULATION: Location Class
// Represents a node in the graph. Data is encapsulated (private members).
// =================================================================================
class Location {
private:
    int id;
    string name;

public:
    Location(int id = 0, string name = "") : id(id), name(name) {}

    int getId() const { return id; }
    string getName() const { return name; }
};

// =================================================================================
// ABSTRACTION & POLYMORPHISM: Abstract Route Class
// This is an abstract base class for different types of routes (edges).
// It uses pure virtual functions to enforce an interface on derived classes.
// =================================================================================
class Route {
protected:
    Location* source;
    Location* destination;
    double distance; // in km
    double time;     // in hours
    double cost;     // in currency

public:
    Route(Location* src, Location* dest, double dist, double t, double c)
        : source(src), destination(dest), distance(dist), time(t), cost(c) {}

    // Virtual destructor for proper cleanup of derived objects through base pointers
    virtual ~Route() {}

    // Pure virtual function makes Route an abstract class
    // This allows for polymorphic behavior based on user preference
    virtual double calculateWeight() const = 0;

    // Getters
    Location* getSource() const { return source; }
    Location* getDestination() const { return destination; }
    double getDistance() const { return distance; }
    double getTime() const { return time; }
    double getCost() const { return cost; }
};


// =================================================================================
// INHERITANCE: Concrete Route Classes
// These classes inherit from the abstract Route class and provide specific
// implementations for calculating the route's weight.
// =================================================================================

class TimeBasedRoute : public Route {
public:
    TimeBasedRoute(Location* src, Location* dest, double dist, double t, double c)
        : Route(src, dest, dist, t, c) {}

    // Specific implementation for time-based preference
    double calculateWeight() const override {
        return time;
    }
};

class CostBasedRoute : public Route {
public:
    CostBasedRoute(Location* src, Location* dest, double dist, double t, double c)
        : Route(src, dest, dist, t, c) {}

    // Specific implementation for cost-based preference
    double calculateWeight() const override {
        return cost;
    }
};

class DistanceBasedRoute : public Route {
public:
    DistanceBasedRoute(Location* src, Location* dest, double dist, double t, double c)
        : Route(src, dest, dist, t, c) {}

    // Specific implementation for distance-based preference
    double calculateWeight() const override {
        return distance;
    }
};


// =================================================================================
// GRAPH CLASS: Manages the network of locations and routes
// =================================================================================
class Graph {
private:
    // Adjacency list: map a location ID to a vector of routes starting from it
    map<int, vector<unique_ptr<Route>>> adjList;
    // Map to hold all locations for easy access
    map<int, Location> locations;
    int nextLocationId = 1;

public:
    // --- CRUD Functionality for Locations ---
    void addLocation(const string& name) {
        int id = nextLocationId++;
        locations[id] = Location(id, name);
        adjList[id] = {}; // Initialize adjacency list for the new location
        cout << "Location '" << name << "' added with ID " << id << "." << endl;
    }

    void readLocation(int id) {
        if (locations.find(id) != locations.end()) {
            cout << "Location ID: " << locations[id].getId() << ", Name: " << locations[id].getName() << endl;
        } else {
            cout << "Location with ID " << id << " not found." << endl;
        }
    }
    
    // --- CRUD Functionality for Routes ---
    void addRoute(int srcId, int destId, double dist, double time, double cost) {
        if (locations.find(srcId) == locations.end() || locations.find(destId) == locations.end()) {
            throw runtime_error("Invalid source or destination ID.");
        }
        
        Location* src = &locations[srcId];
        Location* dest = &locations[destId];
        
        // Example: Adding multiple route types to demonstrate polymorphism
        adjList[srcId].push_back(make_unique<TimeBasedRoute>(src, dest, dist, time, cost));
        // In a real scenario, you'd add the route type based on some logic
        // adjList[srcId].push_back(make_unique<CostBasedRoute>(src, dest, dist, time, cost));

        cout << "Route from " << src->getName() << " to " << dest->getName() << " added." << endl;
    }

    // --- Pathfinding ---
    void findShortestPath(int srcId, int destId, const string& preference) {
        // Placeholder for Dijkstra or A* algorithm
        // The 'preference' string ("time", "cost", "distance") will determine
        // which 'calculateWeight' implementation to use polymorphically.
        cout << "\nFinding shortest path from " << locations[srcId].getName() 
                  << " to " << locations[destId].getName() 
                  << " with preference for lowest " << preference << "." << endl;
        // ... algorithm implementation goes here ...
    }

    // --- Visualization ---
    void visualizeText() {
        cout << "\n--- Graph Visualization ---" << endl;
        for (const auto& pair : adjList) {
            Location loc = locations[pair.first];
            cout << "Location: " << loc.getName() << " (ID: " << loc.getId() << ")" << endl;
            for (const auto& route : pair.second) {
                cout << "  -> " << route->getDestination()->getName()
                          << " (Dist: " << route->getDistance() 
                          << ", Time: " << route->getTime() 
                          << ", Cost: " << route->getCost() << ")" << endl;
            }
        }
        cout << "-------------------------" << endl;
    }

    // --- File I/O ---
    void saveToFile(const string& filename) {
        // Placeholder for saving graph data to a file (e.g., CSV, JSON)
        cout << "Saving graph to " << filename << "..." << endl;
    }

    void loadFromFile(const string& filename) {
        // Placeholder for loading graph data from a file
        cout << "Loading graph from " << filename << "..." << endl;
    }
};

// =================================================================================
// MAIN FUNCTION: Entry point of the program
// =================================================================================
int main() {
    Graph transportationSystem;

    try {
        // --- Setup the graph based on your project data ---
        cout << "Initializing transportation system..." << endl;
        transportationSystem.addLocation("City A"); // ID 1
        transportationSystem.addLocation("City B"); // ID 2
        transportationSystem.addLocation("City C"); // ID 3
        transportationSystem.addLocation("City D"); // ID 4

        // Add routes
        transportationSystem.addRoute(1, 2, 100, 1.5, 50);  // A -> B
        transportationSystem.addRoute(1, 3, 200, 3.0, 120); // A -> C
        transportationSystem.addRoute(2, 3, 50,  0.8, 25);  // B -> C
        transportationSystem.addRoute(2, 4, 150, 2.0, 80);  // B -> D
        transportationSystem.addRoute(3, 4, 180, 2.5, 90);  // C -> D
        
        // --- Demonstrate Features ---
        
        // Visualize the graph
        transportationSystem.visualizeText();
        
        // Find path based on user preference (demonstrates polymorphism)
        transportationSystem.findShortestPath(1, 4, "time");
        transportationSystem.findShortestPath(1, 4, "cost");

        // Save/Load functionality
        transportationSystem.saveToFile("graph_data.txt");
        transportationSystem.loadFromFile("graph_data.txt");

    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << endl;
    }

    return 0;
}