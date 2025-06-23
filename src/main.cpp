#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <fstream>
#include <sstream>
using namespace std;
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
enum class TransportationType { BUS, TRAIN, BOAT, PLANE };
enum class TransportationClass { ECONOMY, EXECUTIVE, BUSINESS, FIRST_CLASS };

/**
 * @struct UserPreferences
 * @brief Encapsulates all user choices into a single object. This is the OUTPUT of the Decision Tree.
 */
struct UserPreferences {
    string profileName; // e.g., "Budget Traveler", "Business Professional"
    double timeWeight = 1.0;
    double costWeight = 1.0;
    double distanceWeight = 1.0;
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

    TreeNode(string q) : question(q) {} // Constructor for question nodes
    TreeNode(UserPreferences prefs) : isLeaf(true), preferences(prefs) {} // Constructor for leaf nodes
};

/**
 * @class DecisionTree
 * @brief Manages and runs the decision-making process to determine UserPreferences.
 */
class DecisionTree {
private:
    unique_ptr<TreeNode> root;

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
     * @brief Constructs the static decision tree with all questions and outcomes.
     */
    void buildTree() {
        // Define final outcomes (leaf nodes)
        auto budgetTraveler = UserPreferences{"Budget Traveler", 1.0, 10.0, 3.0};
        auto balancedTraveler = UserPreferences{"Balanced Traveler", 5.0, 5.0, 5.0};
        auto businessTraveler = UserPreferences{"Business Traveler", 10.0, 2.0, 1.0};
        auto scenicExplorer = UserPreferences{"Scenic Explorer", 2.0, 4.0, 10.0};

        // Create the tree structure from the bottom up
        root = make_unique<TreeNode>("What is your main priority for this trip?");
        
        // Branch 1: Time is most important
        auto timeNode = make_unique<TreeNode>("You prioritize speed. Is budget a major concern?");
        timeNode->children["1. Yes, budget is tight."] = make_unique<TreeNode>(balancedTraveler);
        timeNode->children["2. No, speed is everything."] = make_unique<TreeNode>(businessTraveler);

        // Branch 2: Cost is most important
        auto costNode = make_unique<TreeNode>("You prioritize low cost. Are you completely flexible on time?");
        costNode->children["1. Yes, I have plenty of time."] = make_unique<TreeNode>(budgetTraveler);
        costNode->children["2. No, I still need a reasonable travel time."] = make_unique<TreeNode>(balancedTraveler);
        
        // Branch 3: Scenery/Distance is most important
        auto distanceNode = make_unique<TreeNode>("You want to explore. Do you prefer a direct route or one with more stops?");
        distanceNode->children["1. A direct, scenic route."] = make_unique<TreeNode>(scenicExplorer);
        distanceNode->children["2. A balanced approach is fine."] = make_unique<TreeNode>(balancedTraveler);

        // Connect branches to the root
        root->children["1. Fastest time"] = move(timeNode);
        root->children["2. Lowest cost"] = move(costNode);
        root->children["3. Shortest distance / Most scenic"] = move(distanceNode);
    }

    /**
     * @brief Traverses the tree by asking the user questions to find their preferences.
     * @return The UserPreferences object determined by the user's answers.
     */
    UserPreferences run() const {
        cout << "\n--- Let's Find Your Travel Style! ---\n";
        TreeNode* currentNode = root.get();
        string answer;

        while (!currentNode->isLeaf) {
            cout << "\nQ: " << currentNode->question << endl;
            for (const auto& pair : currentNode->children) {
                cout << "   " << pair.first << endl;
            }
            cout << "Your choice: ";
            cin >> answer;

            if (currentNode->children.count(answer)) {
                currentNode = currentNode->children.at(answer).get();
            } else {
                cout << "Invalid choice, please try again." << endl;
            }
        }

        cout << "\n---------------------------------------\n";
        cout << "Great! We've determined your profile is: " << currentNode->preferences.profileName << endl;
        cout << "---------------------------------------\n";
        return currentNode->preferences;
    }

    /**
     * @brief Prints a textual representation of the decision tree structure.
     */
    void visualize() const {
        cout << "\n--- Decision Tree Structure ---\n";
        printTree(root.get(), 0);
        cout << "-----------------------------\n";
    }
};


// =================================================================================
// 3. GRAPH & ROUTING IMPLEMENTATION (Largely unchanged)
// =================================================================================

class Location { /* ... same as before ... */
private: int id; string name; double latitude; double longitude;
public: Location(int id=0, string n="", double lat=0.0, double lon=0.0):id(id),name(n),latitude(lat),longitude(lon){}
int getId() const {return id;} string getName() const {return name;}
};

class Route { /* ... same as before ... */
protected: Location* source; Location* destination; double distance; double time; double cost;
public: Route(Location* s, Location* d, double dist, double t, double c):source(s),destination(d),distance(dist),time(t),cost(c){}
virtual ~Route(){} virtual double calculateWeight(const UserPreferences& p) const = 0;
Location* getDestination() const { return destination; }
};

class ConcreteRoute : public Route { /* ... same as before ... */
public: ConcreteRoute(Location* s, Location* d, double dist, double t, double c):Route(s,d,dist,t,c){}
double calculateWeight(const UserPreferences& prefs) const override {
    // NOTE: In a real system, you would NORMALIZE these values first!
    return (time * prefs.timeWeight) + (cost * prefs.costWeight) + (distance * prefs.distanceWeight);
}
};

class Graph { /* ... same as before, simplified for clarity ... */
private: map<int, Location> locations; map<int, vector<unique_ptr<Route>>> adjList; int nextId=1;
public:
    void addLocation(const string& n, double lat, double lon){ int id=nextId++; locations[id]=Location(id,n,lat,lon); adjList[id]={}; }
    void addRoute(int sId, int dId, double d, double t, double c){
        if(!locations.count(sId) || !locations.count(dId)) return;
        adjList.at(sId).push_back(make_unique<ConcreteRoute>(&locations.at(sId),&locations.at(dId),d,t,c));
    }
    void findShortestPath(int startId, int goalId, const UserPreferences& prefs) const {
        cout << "\n======================================================\n";
        cout << "Finding route for profile: " << prefs.profileName << "\n";
        cout << "Searching path from " << locations.at(startId).getName() << " to " << locations.at(goalId).getName() << "...\n";
        cout << "A* algorithm would run here using the calculated weights.\n";
        cout << "======================================================\n\n";
    }
};


// =================================================================================
// 4. MAIN PROGRAM FLOW
// =================================================================================

int main() {
    try {
        // --- Step 1: Setup the Graph and Decision Tree ---
        Graph transportationSystem;
        DecisionTree preferenceFinder;

        // Populate the graph (this would normally be from a file)
        transportationSystem.addLocation("Jakarta (JKT)", -6.17, 106.82);
        transportationSystem.addLocation("Bandung (BDO)", -6.91, 107.61);
        transportationSystem.addLocation("Surabaya (SUB)", -7.25, 112.75);
        transportationSystem.addRoute(1, 2, 150, 3.0, 150000); // JKT -> BDO
        transportationSystem.addRoute(1, 3, 780, 1.5, 900000); // JKT -> SUB (Plane)
        transportationSystem.addRoute(2, 3, 650, 10.0, 400000); // BDO -> SUB

        // --- Step 2: Visualize the Decision Tree Structure (as required) ---
        preferenceFinder.visualize();

        // --- Step 3: Run the Decision Tree to get user's preferences ---
        UserPreferences userPrefs = preferenceFinder.run();

        // --- Step 4: Use the determined preferences to find the best route ---
        // The `userPrefs` object is the bridge between the two data structures.
        transportationSystem.findShortestPath(1, 3, userPrefs); // Find path from Jakarta to Surabaya

    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << endl;
        return 1;
    }

    return 0;
}
