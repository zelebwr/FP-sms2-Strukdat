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

// Added to remove the "std::" prefix from standard library components
using namespace std;

// Define M_PI if it's not defined by the cmath header on some compilers
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =================================================================================
// 1. CORE DATA TYPES & FORWARD DECLARATIONS
// =================================================================================

class Location;
class Route;
class Graph;
struct UserPreferences;
struct TreeNode;
class DecisionTree;

enum class TransportationType { ANY, BUS, TRAIN, BOAT, PLANE };

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

struct UserPreferences {
    string profileName;
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
// =================================================================================

struct TreeNode {
    string question; bool isLeaf = false; UserPreferences preferences;
    // THE FIX - ALTERNATIVE: Using shared_ptr for the tree nodes as well for consistency.
    map<string, shared_ptr<TreeNode>> children;
    TreeNode(string q) : question(move(q)) {} TreeNode(UserPreferences prefs) : isLeaf(true), preferences(move(prefs)) {}
};

class DecisionTree {
private:
    shared_ptr<TreeNode> root;
    void buildTree() {
        auto budget=UserPreferences{"Budget Traveler",1.0,10.0,3.0}; auto balanced=UserPreferences{"Balanced Traveler",5.0,5.0,5.0}; auto business=UserPreferences{"Business Traveler",10.0,2.0,1.0};
        root = make_shared<TreeNode>("What is your main priority?");
        root->children["1. Fastest Time"]=make_shared<TreeNode>(business); root->children["2. Lowest Cost"]=make_shared<TreeNode>(budget); root->children["3. A Balanced Approach"]=make_shared<TreeNode>(balanced);
    }
    void printTree(const shared_ptr<TreeNode>& n, int i) const { if(!n)return; for(int k=0;k<i;++k)cout<<"  "; if(n->isLeaf){cout<<"-> LEAF: "<<n->preferences.profileName<<"\n";}else{cout<<"Q: "<<n->question<<"\n"; for(const auto& p:n->children){for(int k=0;k<i;++k)cout<<"  "; cout<<"  ["<<p.first<<"]\n"; printTree(p.second,i+2);}}}

public:
    DecisionTree(){buildTree();}
        UserPreferences run() const {
        cout<<"\n--- Let's Find Your Travel Style! ---\n";
        const TreeNode* currentNode = root.get();
        string answer;

        while(!currentNode->isLeaf){
            cout<<"\nQ: "<<currentNode->question<<"\n";
            for(const auto& p : currentNode->children){ cout<<"   "<<p.first<<"\n"; }
            cout<<"Your choice: ";
            cin>>answer;

            // THE FIX: Instead of an exact match, find the key that STARTS WITH the user's answer.
            string matchedKey = "";
            for (const auto& pair : currentNode->children) {
                // `rfind` checks if the key starts with the answer string.
                if (pair.first.rfind(answer, 0) == 0) {
                    matchedKey = pair.first;
                    break;
                }
            }
            
            if (!matchedKey.empty()) {
                currentNode = currentNode->children.at(matchedKey).get();
            } else {
                cout<<"Invalid choice, please try again.\n";
            }
        }
        
        UserPreferences finalPrefs = currentNode->preferences;
        cout<<"\nProfile set to: "<<finalPrefs.profileName<<"\n";
        
        // Applying the same fix for the second question
        map<string, TransportationType> transportChoices = {
            {"1. Train", TransportationType::TRAIN},
            {"2. Bus", TransportationType::BUS},
            {"3. Plane", TransportationType::PLANE},
            {"4. Boat", TransportationType::BOAT},
            {"5. Any (no preference)", TransportationType::ANY}
        };

        bool choiceMade = false;
        while (!choiceMade) {
            cout << "\nWhich mode of transport do you prefer?\n";
            for (const auto& pair : transportChoices) {
                cout << "   " << pair.first << endl;
            }
            cout << "Your choice: ";
            cin >> answer;
            
            string matchedKey = "";
            for (const auto& pair : transportChoices) {
                if (pair.first.rfind(answer, 0) == 0) {
                    matchedKey = pair.first;
                    break;
                }
            }

            if (!matchedKey.empty()) {
                finalPrefs.preferredTransport = transportChoices.at(matchedKey);
                choiceMade = true;
            } else {
                cout << "Invalid choice, please try again.\n";
            }
        }

        cout << "Transportation preference set to: " << transportTypeToString(finalPrefs.preferredTransport) << endl;
        
        return finalPrefs;
    }

    void visualize() const {
        cout<<"\n--- Decision Tree Structure ---\n"; 
        printTree(root, 0); cout<<"-----------------------------\n";
    }
};


// =================================================================================
// 3. GRAPH & ROUTING IMPLEMENTATION
// =================================================================================

class Location {
private: int id; string name; double latitude; double longitude;
public: Location(int id=0, string name="N/A", double lat=0.0, double lon=0.0):id(id),name(name),latitude(lat),longitude(lon){}
int getId() const {return id;} string getName() const {return name;} double getLatitude() const {return latitude;} double getLongitude() const {return longitude;}
};

class Route {
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

class ConcreteRoute : public Route {
public:
    ConcreteRoute(Location* s, Location* d, double dist, double t, double c, TransportationType tp)
        : Route(s, d, dist, t, c, tp) {}

    double calculateWeight(const UserPreferences& prefs, const GraphStats& stats) const override {
        double normTime = time / stats.maxTime;
        double normCost = cost / stats.maxCost;
        double normDist = distance / stats.maxDistance;
        double baseWeight = (normTime * prefs.timeWeight) + (normCost * prefs.costWeight) + (normDist * prefs.distanceWeight);
        if (prefs.preferredTransport != TransportationType::ANY && this->type != prefs.preferredTransport) {
            baseWeight += 1000.0;
        }
        return baseWeight;
    }
};

class Graph {
private:
    map<int, Location> locations;
    // THE FIX - ALTERNATIVE: Using shared_ptr instead of unique_ptr.
    // shared_ptr is copyable and will resolve the compilation error.
    map<int, vector<shared_ptr<Route>>> adjList;
    GraphStats stats;
    int nextId = 1;

    double heuristic(const Location& a, const Location& b) const {
        const double R=6371.0; double lat1=a.getLatitude()*M_PI/180.0; double lon1=a.getLongitude()*M_PI/180.0; double lat2=b.getLatitude()*M_PI/180.0; double lon2=b.getLongitude()*M_PI/180.0;
        double dlon=lon2-lon1; double dlat=lat2-lat1; double val=pow(sin(dlat/2),2)+cos(lat1)*cos(lat2)*pow(sin(dlon/2),2); double c=2*asin(sqrt(val)); return R*c;
    }

    void reconstructAndPrintPath(const map<int, int>& came_from, int currentId) const {
        vector<int> total_path; total_path.push_back(currentId);
        while (came_from.count(currentId)) { currentId = came_from.at(currentId); total_path.push_back(currentId); }
        reverse(total_path.begin(), total_path.end());

        cout << "\nBest Route Found:\n";
        double totalTime = 0, totalCost = 0, totalDist = 0;
        for (size_t i = 0; i < total_path.size() - 1; ++i) {
            int fromId = total_path[i]; int toId = total_path[i+1]; shared_ptr<const Route> routeTaken = nullptr;
            for (const auto& r : adjList.at(fromId)) { if (r->getDestination()->getId() == toId) { routeTaken = r; break; } }
            if (routeTaken) {
                cout << "  " << i + 1 << ". From " << locations.at(fromId).getName()
                          << " to " << locations.at(toId).getName()
                          << " by " << transportTypeToString(routeTaken->getType())
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
    // No longer need to delete the copy constructor with shared_ptr
    Graph() = default;

    void printAllLocations() const {
        cout << "\n--- Available Locations ---\n";
        for (const auto& pair : locations) {
            cout << "ID: " << pair.first << " -> " << pair.second.getName() << endl;
        }
        cout << "---------------------------\n";
    }

    bool isValidLocation(int id) const {
        return locations.count(id);
    }

    void addLocation(const string& name, double lat, double lon) { int id=nextId++; locations[id]=Location(id,name,lat,lon); adjList[id]={}; }

    void addRoute(int srcId, int destId, double dist, double time, double cost, TransportationType type) {
        if (!locations.count(srcId) || !locations.count(destId)) return;
        if (type == TransportationType::ANY) {
            throw runtime_error("A specific route must have a specific transportation type, not ANY.");
        }
        // THE FIX - ALTERNATIVE: Use make_shared instead of make_unique
        adjList.at(srcId).push_back(make_shared<ConcreteRoute>(&locations.at(srcId), &locations.at(destId), dist, time, cost, type));

        if (time > stats.maxTime) stats.maxTime = time;
        if (cost > stats.maxCost) stats.maxCost = cost;
        if (dist > stats.maxDistance) stats.maxDistance = dist;
    }

    void findShortestPath(int startId, int goalId, const UserPreferences& prefs) const {
        using QueueElement = pair<double, int>;
        priority_queue<QueueElement, vector<QueueElement>, greater<QueueElement>> openSet;
        map<int, int> came_from; map<int, double> g_score;
        for (const auto& pair : locations) { g_score[pair.first] = numeric_limits<double>::infinity(); }
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

        transportationSystem.addLocation("Jakarta (JKT)", -6.1751, 106.8650);
        transportationSystem.addLocation("Bandung (BDO)", -6.9175, 107.6191);
        transportationSystem.addLocation("Surabaya (SUB)", -7.2575, 112.7521);
        transportationSystem.addLocation("Denpasar (DPS)", -8.6700, 115.2124);
        transportationSystem.addLocation("Semarang (SRG)", -6.9667, 110.4381);
        transportationSystem.addLocation("Makassar (MKS)", -5.1354, 119.4238);

        transportationSystem.addRoute(1, 2, 150000.0, 180.0, 150.0, TransportationType::TRAIN);
        transportationSystem.addRoute(1, 5, 500000.0, 360.0, 300.0, TransportationType::TRAIN);
        transportationSystem.addRoute(2, 3, 650000.0, 600.0, 400.0, TransportationType::TRAIN);
        transportationSystem.addRoute(5, 3, 350000.0, 300.0, 150.5, TransportationType::BUS);
        transportationSystem.addRoute(3, 4, 430000.0, 60.0, 700.0, TransportationType::PLANE);
        transportationSystem.addRoute(1, 4, 1180000.0, 114.0, 1200.0, TransportationType::PLANE);
        transportationSystem.addRoute(3, 6, 620000.0, 1860.0, 450.0, TransportationType::BOAT);

        // --- Get User Preferences ---
        preferenceFinder.visualize();
        UserPreferences userPrefs = preferenceFinder.run();

        // --- MODIFIED: Get User Start and End Locations ---
        int startId = 0, goalId = 0;
        transportationSystem.printAllLocations();

        while (true) {
            cout << "\nEnter the ID for your starting location: ";
            cin >> startId;
            if (transportationSystem.isValidLocation(startId)) {
                break;
            }
            cout << "Invalid ID. Please try again." << endl;
        }

        while (true) {
            cout << "Enter the ID for your destination: ";
            cin >> goalId;
            if (transportationSystem.isValidLocation(goalId)) {
                break;
            }
            cout << "Invalid ID. Please try again." << endl;
        }

        // --- Find and Display the Path ---
        transportationSystem.findShortestPath(startId, goalId, userPrefs);

    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << endl;
        return 1;
    }
    return 0;
}