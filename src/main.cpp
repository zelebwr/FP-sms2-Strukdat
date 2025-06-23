#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;

// Define M_PI if it's not defined by the cmath header on some compilers
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =================================================================================
// 1. FORWARD DECLARATIONS & CORE DATA TYPES
// =================================================================================

// Forward declare classes so they can reference each other
class Location;
class Route;
class Graph;
struct UserPreferences;
struct TreeNode; // Node for our Decision Tree
class DecisionTree;

// Using enums makes the code safer and more readable
enum class TransportationType { ANY, BUS, TRAIN, BOAT, PLANE };

// Helper to convert enum to string for printing
string transportTypeToString(TransportationType type) {
    switch (type) {
        case TransportationType::BUS: return "Bus";
        case TransportationType::TRAIN: return "Train";
        case TransportationType::BOAT: return "Boat";
        case TransportationType::PLANE: return "Plane";
        case TransportationType::ANY: return "Any";
    }
    return "Unknown";
}

/**
 * @struct UserPreferences
 * @brief Encapsulates all user choices into a single object. This is the OUTPUT of the Decision Tree.
 */
struct UserPreferences {
    string profileName; // e.g., "Budget Traveler", "Business Professional"
    double timeWeight = 1.0;
    double costWeight = 1.0;
    double distanceWeight = 1.0;
    TransportationType preferredTransport = TransportationType::ANY;
};

struct GraphStats {
    double maxTime = 1.0;
    double maxCost = 1.0;
    double maxDistance = 1.0;
};

// =================================================================================
// 2. DECISION TREE IMPLEMENTATION
// This section implements the Tree data structure for determining user preferences.
// =================================================================================

/**
 * @struct TreeNode
 * @brief Represents a node in the Decision Tree. It's either a question or a final decision (a leaf).
 */
struct TreeNode {
    string question;
    bool isLeaf = false;
    UserPreferences preferences; // Only used if it's a leaf node

    // Maps an answer (e.g., "1", "2") to the next node in the tree.
    // Using unique_ptr for automatic memory management of child nodes.
    map<string, unique_ptr<TreeNode>> children;

    TreeNode(string q) : question(move(q)) {} // Constructor for question nodes
    TreeNode(UserPreferences prefs) : isLeaf(true), preferences(move(prefs)) {} // Constructor for leaf nodes
};

/**
 * @class DecisionTree
 * @brief Manages and runs the decision-making process to determine UserPreferences.
 */
class DecisionTree {
private:
    unique_ptr<TreeNode> root;

    void buildTree() {
        auto budget = UserPreferences{"Budget Traveler", 1.0, 10.0, 3.0};
        auto balanced = UserPreferences{"Balanced Traveler", 5.0, 5.0, 5.0};
        auto business = UserPreferences{"Business Traveler", 10.0, 2.0, 1.0};
        
        root = make_unique<TreeNode>("What is your main priority for this trip?");
        root->children["1. Fastest Time"] = make_unique<TreeNode>(business);
        root->children["2. Lowest Cost"] = make_unique<TreeNode>(budget);
        root->children["3. A Balanced Approach"] = make_unique<TreeNode>(balanced);
    }

    // A helper function for visualizing the tree recursively
    void printTree(const TreeNode* node, int indent) const {
        if (!node) return;

        // Print indentation
        for (int i = 0; i < indent; ++i) cout << "  ";

        if (node->isLeaf) {
            cout << "-> LEAF: " << node->preferences.profileName << " (Time:"
                      << node->preferences.timeWeight << ", Cost:" << node->preferences.costWeight << ")" << endl;
        } else {
            cout << "Q: " << node->question << endl;
            for (const auto& pair : node->children) {
                for (int i = 0; i < indent; ++i) cout << "  ";
                cout << "  [" << pair.first << "] " << endl;
                printTree(pair.second.get(), indent + 2);
            }
        }
    }

public:
    DecisionTree() {
        buildTree();
    }

    /**
     * @brief Traverses the tree by asking the user questions to find their preferences.
     * @return The UserPreferences object determined by the user's answers.
     */
    UserPreferences run() const {
        cout<<"\n--- Let's Find Your Travel Style! ---\n";
        const TreeNode* currentNode = root.get();
        string answer;

        while(!currentNode->isLeaf){
            cout<<"\nQ: "<<currentNode->question<<"\n";
            for(const auto& p : currentNode->children){ cout<<"   "<<p.first<<"\n"; }
            cout<<"Your choice: ";
            cin>>answer;
            if(currentNode->children.count(answer)){ currentNode = currentNode->children.at(answer).get(); }
            else{ cout<<"Invalid choice, try again.\n"; }
        }
        
        UserPreferences finalPrefs = currentNode->preferences;
        cout<<"\nProfile set to: "<<finalPrefs.profileName<<"\n";

        cout << "\nWhich mode of transport do you prefer?\n";
        cout << "1. Train\n";
        cout << "2. Bus\n";
        cout << "3. Plane\n";
        cout << "4. Any (no preference)\n";
        cout << "Your choice: ";
        cin >> answer;

        if (answer == "1") finalPrefs.preferredTransport = TransportationType::TRAIN;
        else if (answer == "2") finalPrefs.preferredTransport = TransportationType::BUS;
        else if (answer == "3") finalPrefs.preferredTransport = TransportationType::PLANE;
        else finalPrefs.preferredTransport = TransportationType::ANY;

        cout << "Transportation preference set to: " << transportTypeToString(finalPrefs.preferredTransport) << endl;
        
        return finalPrefs;
    }
    void visualize() const {cout<<"\n--- Decision Tree Structure ---\n"; printTree(root.get(),0); cout<<"-----------------------------\n";}
};


// =================================================================================
// 3. GRAPH & ROUTING IMPLEMENTATION (Largely unchanged)
// =================================================================================

class Location { /* ... same as before ... */
private: 
    int id; 
    string name; 
    double latitude; 
    double longitude;
public: 
    Location(int id=0, string n="", double lat=0.0, double lon=0.0)
        :id(id),name(n),latitude(lat),longitude(lon){}
    int getId() const {return id;}
    string getName() const {return name;}
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }
};

class Route { /* ... same as before ... */
protected: 
    Location* source; 
    Location* destination; 
    double distance; 
    double time; 
    double cost;
    TransportationType type;

public: 
    Route(Location* s, Location* d, double dist, double t, double c, TransportationType tp)
        : source(s), destination(d), distance(dist), time(t), cost(c), type(tp) {}
    virtual ~Route() = default;
    virtual double calculateWeight(const UserPreferences& p, const GraphStats& stats) const = 0;
    Location* getDestination() const { return destination; }
    double getDistance() const { return distance; }
    double getTime() const { return time; }
    double getCost() const { return cost; }
    TransportationType getType() const { return type; }
};

class ConcreteRoute : public Route { /* ... same as before ... */
public: 
    ConcreteRoute(Location* s, Location* d, double dist, double t, double c, TransportationType tp)
        : Route(s, d, dist, t, c, tp) {}

    double calculateWeight(const UserPreferences& prefs, const GraphStats& stats) const override {
        double normTime = time / stats.maxTime;
        double normCost = cost / stats.maxCost;
        double normDist = distance / stats.maxDistance;

        double baseWeight = (normTime * prefs.timeWeight) + (normCost * prefs.costWeight) + (normDist * prefs.distanceWeight);

        if (prefs.preferredTransport != TransportationType::ANY && this->type != prefs.preferredTransport) {
            baseWeight += 1000.0; // A large penalty to deprioritize this route
        }

        return baseWeight;
    }
};

class Graph { /* ... same as before, simplified for clarity ... */
private: 
    map<int, Location> locations;
    map<int, vector<unique_ptr<Route>>> adjList;
    GraphStats stats;
    int nextId = 1;

    double heuristic(const Location& a, const Location& b) const {
        const double R=6371.0; 
        double lat1=a.getLatitude()*M_PI/180.0; 
        double lon1=a.getLongitude()*M_PI/180.0; 
        double lat2=b.getLatitude()*M_PI/180.0; 
        double lon2=b.getLongitude()*M_PI/180.0;
        double dlon=lon2-lon1; 
        double dlat=lat2-lat1; 
        double val=pow(sin(dlat/2),2)+cos(lat1)*cos(lat2)*pow(sin(dlon/2),2); 
        double c=2*asin(sqrt(val)); 
        return R*c;
    }

    void reconstructAndPrintPath(const map<int, int>& came_from, int currentId) const {
        vector<int> total_path; total_path.push_back(currentId);
        while (came_from.count(currentId)) { currentId = came_from.at(currentId); total_path.push_back(currentId); }
        reverse(total_path.begin(), total_path.end());

        cout << "\nBest Route Found:\n";
        double totalTime = 0, totalCost = 0, totalDist = 0;
        for (size_t i = 0; i < total_path.size() - 1; ++i) {
            int fromId = total_path[i]; int toId = total_path[i+1]; const Route* routeTaken = nullptr;
            for (const auto& r : adjList.at(fromId)) { if (r->getDestination()->getId() == toId) { routeTaken = r.get(); break; } }
            if (routeTaken) {
                cout << "  " << i + 1 << ". From " << locations.at(fromId).getName()
                          << " to " << locations.at(toId).getName()
                          << " by " << transportTypeToString(routeTaken->getType()) // MODIFIED: Show transport type
                          << " (Time: " << routeTaken->getTime() << "m, Cost: " << routeTaken->getCost() << "k"
                          << ", Dist: " << routeTaken->getDistance() << "m)\n";
                totalTime += routeTaken->getTime(); totalCost += routeTaken->getCost(); totalDist += routeTaken->getDistance();
            }
        }
        cout << "------------------------------------------------------\n";
        cout << "Total Trip -> Time: " << totalTime << " minutes, "
                  << "Cost: Rp " << fixed << setprecision(3) << (totalCost * 1000)
                  << ", Distance: " << totalDist << " meters\n";
        cout << "======================================================\n\n";
    }
    
public:
    // This tells the compiler that the Graph object cannot be copied.
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    // We must define the default constructor since we deleted the copy constructor
    Graph() = default;

    void addLocation(const string& name, double lat, double lon) {
        int id = nextId++;
        locations[id] = Location(id, name, lat, lon);
        adjList[id] = {};
    }

    void addRoute(int srcId, int destId, double dist, double time, double cost, TransportationType type) {
        if (!locations.count(srcId) || !locations.count(destId)) return;
        adjList.at(srcId).push_back(make_unique<ConcreteRoute>(&locations.at(srcId), &locations.at(destId), dist, time, cost, type));

        if (time > stats.maxTime) stats.maxTime = time;
        if (cost > stats.maxCost) stats.maxCost = cost;
        if (dist > stats.maxDistance) stats.maxDistance = dist;
    }

    void findShortestPath(int startId, int goalId, const UserPreferences& prefs) const {
        using QueueElement = pair<double, int>;
        priority_queue<QueueElement, vector<QueueElement>, greater<QueueElement>> openSet;
        map<int, int> came_from;
        map<int, double> g_score;
        for (const auto& pair : locations) { 
            g_score[pair.first] = numeric_limits<double>::infinity(); 
        }
        g_score[startId] = 0;
        openSet.push({heuristic(locations.at(startId), locations.at(goalId)), startId});
        cout << "\n======================================================\n";
        cout << "Searching path for profile: " << prefs.profileName << " (" << transportTypeToString(prefs.preferredTransport) << " preferred)...\n";
        while (!openSet.empty()) {
            int currentId = openSet.top().second; openSet.pop();
            if (currentId == goalId) { reconstructAndPrintPath(came_from, currentId); return; }
            if (adjList.count(currentId)) {
                for (const auto& route : adjList.at(currentId)) {
                    int neighborId = route->getDestination()->getId();
                    double tentative_g_score = g_score.at(currentId) + route->calculateWeight(prefs, stats);
                    if (tentative_g_score < g_score.at(neighborId)) {
                        came_from[neighborId] = currentId;
                        g_score[neighborId] = tentative_g_score;
                        double f_score = g_score.at(neighborId) + heuristic(locations.at(neighborId), locations.at(goalId));
                        openSet.push({f_score, neighborId});
                    }
                }
            }
        }
        cout << "No path found from " << locations.at(startId).getName() << " to " << locations.at(goalId).getName() << ".\n";
    }
};

// =================================================================================
// 4. MAIN PROGRAM FLOW
// =================================================================================

int main() {
    try {
        Graph transportationSystem;
        DecisionTree preferenceFinder;

        // --- Setup the Graph with sample data (MODIFIED to include transport types) ---
        transportationSystem.addLocation("Jakarta (JKT)", -6.1751, 106.8650);
        transportationSystem.addLocation("Bandung (BDO)", -6.9175, 107.6191);
        transportationSystem.addLocation("Surabaya (SUB)", -7.2575, 112.7521);
        transportationSystem.addLocation("Denpasar (DPS)", -8.6700, 115.2124);
        transportationSystem.addLocation("Semarang (SRG)", -6.9667, 110.4381);

        // time in minutes, distance in meters, cost in Thousand Rupiah
        transportationSystem.addRoute(1, 2, 150000.0, 180.0, 150.0, TransportationType::TRAIN);
        transportationSystem.addRoute(1, 5, 500000.0, 360.0, 300.0, TransportationType::TRAIN);
        transportationSystem.addRoute(2, 3, 650000.0, 600.0, 400.0, TransportationType::TRAIN);
        transportationSystem.addRoute(5, 3, 350000.0, 300.0, 150.5, TransportationType::BUS);
        transportationSystem.addRoute(3, 4, 430000.0, 60.0, 700.0, TransportationType::PLANE);
        transportationSystem.addRoute(1, 4, 1180000.0, 114.0, 1200.0, TransportationType::PLANE);


        preferenceFinder.visualize();
        UserPreferences userPrefs = preferenceFinder.run();
        transportationSystem.findShortestPath(1, 4, userPrefs);

    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << endl;
        return 1;
    }

    return 0;
}
