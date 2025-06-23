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
#include <filesystem>

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
class FileManager;

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

TransportationType stringToTransportType(const string& s) {
    if (s == "TRAIN") return TransportationType::TRAIN;
    if (s == "BUS") return TransportationType::BUS;
    if (s == "BOAT") return TransportationType::BOAT;
    if (s == "PLANE") return TransportationType::PLANE;
    return TransportationType::ANY; // Default case
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
private: 
    int id; 
    string name; 
    double latitude; 
    double longitude;
public: 
    Location(int id=0, string name="N/A", double lat=0.0, double lon=0.0)
        :id(id),name(name),latitude(lat),longitude(lon){}
    int getId() const {return id;} 
    string getName() const {return name;} 
    double getLatitude() const {return latitude;} 
    double getLongitude() const {return longitude;}
    void update(string n, double lat, double lon){name=n; latitude=lat; longitude=lon;}
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
    Location* getSource() const { return source; }
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
    map<int, vector<shared_ptr<Route>>> adjList;
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

    vector<shared_ptr<const Route>> reconstructPath(const map<int, int>& came_from, int currentId) const {
        vector<shared_ptr<const Route>> path;
        int toId = currentId;
        while (came_from.count(toId)) {
            int fromId = came_from.at(toId);
            shared_ptr<const Route> routeTaken = nullptr;
            for (const auto& r : adjList.at(fromId)) {
                if (r->getDestination()->getId() == toId) {
                    routeTaken = r;
                    break;
                }
            }
            if (routeTaken) {
                path.push_back(routeTaken);
            }
            toId = fromId;
        }
        reverse(path.begin(), path.end());
        return path;
    }

public:
    // No longer need to delete the copy constructor with shared_ptr
    Graph() = default;

    // --- Getters for File Saving ---
    const map<int, Location>& getAllLocations() const { return locations; }
    const map<int, vector<shared_ptr<Route>>>& getAdjList() const { return adjList; }

    // --- In-Memory CRUD Operations ---
    int addLocation(const string& name, double lat, double lon) {
        int id = nextId++; locations[id] = Location(id, name, lat, lon); adjList[id] = {}; return id;
    }

    void addRoute(int srcId, int destId, double dist, double time, double cost, TransportationType type) {
        if (!locations.count(srcId) || !locations.count(destId)) return;
        if (type == TransportationType::ANY) {
            throw runtime_error("Route must have a specific transport type."); 
        }
        adjList.at(srcId).push_back(make_shared<ConcreteRoute>(&locations.at(srcId), &locations.at(destId), dist, time, cost, type));
        if(time > stats.maxTime) stats.maxTime = time;
        if(cost > stats.maxCost) stats.maxCost = cost; 
        if(dist > stats.maxDistance) stats.maxDistance = dist;
    }

    bool updateLocation(int id, const string& name, double lat, double lon) {
        if (!locations.count(id)) return false;
        locations.at(id).update(name, lat, lon); return true;
    }

    bool deleteRoute(int srcId, int destId) {
        if (!adjList.count(srcId)) return false;
        auto& routes = adjList.at(srcId);
        auto original_size = routes.size();
        routes.erase(remove_if(routes.begin(), routes.end(), [destId](const shared_ptr<Route>& r){
            return r->getDestination()->getId() == destId;
        }), routes.end());
        return routes.size() < original_size;
    }

    bool deleteLocation(int id) {
        if (!locations.count(id)) {
            return false; // Return false if the location doesn't exist.
        }
        locations.erase(id); // Remove the location itself
        adjList.erase(id); // Remove all routes starting from this location
        // Remove all routes pointing to this location
        for (auto& pair : adjList) {
            auto& routes = pair.second;
            routes.erase(remove_if(routes.begin(), routes.end(), [id](const shared_ptr<Route>& route) {
                return route->getDestination()->getId() == id;
            }), routes.end());
        }
        return true; // Return true to indicate success.
    }
    
    // --- Helper and Pathfinding Methods ---
    void printAllLocations() const { /* ... same as before ... */
        cout << "\n--- Available Locations ---\n";
        for (const auto& pair : locations) { cout << "ID: " << pair.first << " -> " << pair.second.getName() << endl; }
        cout << "---------------------------\n";
    }

    void printAllRoutes() const {
        cout << "\n--- All Routes ---\n";
        bool routesExist = false;
        for (const auto& pair : adjList) {
            if (!pair.second.empty()) {
                routesExist = true;
                cout << "Routes from " << locations.at(pair.first).getName() << ":" << endl;
                for (const auto& route : pair.second) {
                    cout << "  -> To: " << route->getDestination()->getName()
                         << " by " << transportTypeToString(route->getType())
                         << " (Cost: " << route->getCost() << "k)" << endl;
                }
            }
        }
        if (!routesExist) {
            cout << "No routes loaded.\n";
        }
        cout << "--------------------\n";
    }

    bool isValidLocation(int id) const { return locations.count(id); }

    // MODIFIED: Returns a path object instead of just printing
    vector<shared_ptr<const Route>> findShortestPath(int startId, int goalId, const UserPreferences& prefs) const {
        using QueueElement = pair<double, int>;
        priority_queue<QueueElement, vector<QueueElement>, greater<QueueElement>> openSet;
        map<int, int> came_from; 
        map<int, double> g_score;
        for (const auto& pair : locations) { 
            g_score[pair.first] = numeric_limits<double>::infinity(); 
        }
        g_score[startId] = 0;
        openSet.push({heuristic(locations.at(startId), locations.at(goalId)), startId});
        
        cout << "\nSearching path for profile: " << prefs.profileName << " (" << transportTypeToString(prefs.preferredTransport) << " preferred)...\n";
        
        while (!openSet.empty()) {
            int currentId = openSet.top().second; openSet.pop();
            if (currentId == goalId) {
                cout << "Path found!" << endl;
                return reconstructPath(came_from, currentId);
            }
            if (adjList.count(currentId)) {
                for (const auto& route : adjList.at(currentId)) {
                    int neighborId = route->getDestination()->getId();
                    double tentative_g_score = g_score.at(currentId) + route->calculateWeight(prefs, stats);
                    if (tentative_g_score < g_score.at(neighborId)) {
                        came_from[neighborId] = currentId; g_score[neighborId] = tentative_g_score;
                        double f_score = g_score.at(neighborId) + heuristic(locations.at(neighborId), locations.at(goalId));
                        openSet.push({f_score, neighborId});
                    }
                }
            }
        }
        cout << "No path found from " << locations.at(startId).getName() << " to " << locations.at(goalId).getName() << ".\n";
        return {}; // Return an empty vector if no path is found
    }
};

class FileManager {
private:
    int findNextFileNumber(const string& prefix, const string& extension) {
        int maxNum = 0;
        for (const auto& entry : filesystem::directory_iterator(".")) {
            string filename = entry.path().filename().string();
            if (filename.rfind(prefix, 0) == 0) {
                try {
                    size_t start = prefix.length();
                    size_t end = filename.find(extension);
                    int num = stoi(filename.substr(start, end - start));
                    if (num > maxNum) maxNum = num;
                } catch (...) { /* ignore files that don't match pattern */ }
            }
        }
        return maxNum + 1;
    }

public:
    void loadAllData(Graph& g) { /* ... same as before ... */
        cout<<"\n--- Loading Data ---\n";
        for(const auto&e:filesystem::directory_iterator(".")){
            string fn=e.path().filename().string();
            if(fn.rfind("input_locations_",0)==0){
                ifstream f(fn);
                if(!f.is_open()){
                    cerr<<"Warn: No "<<fn<<endl;
                    continue;
                }
                string l;
                getline(f,l);
                while(getline(f,l)){
                    stringstream ss(l);
                    string i,n,la,lo;
                    getline(ss,i,';');
                    getline(ss,n,';');
                    getline(ss,la,';');
                    getline(ss,lo,';');
                    if(!n.empty()){
                        g.addLocation(n,stod(la),stod(lo));
                    }
                }
                cout<<"Loaded "<<fn<<endl;
            }
            else if(fn.rfind("input_routes_",0)==0){
                ifstream f(fn);
                if(!f.is_open()){
                    cerr<<"Warn: No "<<fn<<endl;
                    continue;
                }
                string l;
                getline(f,l);
                while(getline(f,l)){
                    stringstream ss(l);
                    string s,d,di,ti,c,ty;
                    getline(ss,s,';');
                    getline(ss,d,';');
                    getline(ss,di,';');
                    getline(ss,ti,';');
                    getline(ss,c,';');
                    getline(ss,ty,';');
                    if(!s.empty()){
                        g.addRoute(stoi(s),stoi(d),stod(di),stod(ti),stod(c),stringToTransportType(ty));
                    }
                }
                cout<<"Loaded "<<fn<<endl;
            }
        } 
        cout<<"--------------------\n";
    }

    void saveLocationsToCSV(const Graph& g, const string& filename) {
        ofstream file(filename);
        file << "id;name;latitude;longitude\n";
        for (const auto& [id, loc] : g.getAllLocations()) {
            file << id << ";" << loc.getName() << ";" << fixed << setprecision(4) << loc.getLatitude() << ";" << loc.getLongitude() << "\n";
        }
        cout << "Locations saved to " << filename << endl;
    }
    
    void saveRoutesToCSV(const Graph& g, const string& filename) {
        ofstream file(filename);
        file << "source_id;dest_id;distance_m;time_min;cost_k_rp;type\n";
        for (const auto& [fromId, routes] : g.getAdjList()) {
            for (const auto& r : routes) {
                file << fromId << ";" << r->getDestination()->getId() << ";" << r->getDistance() << ";" << r->getTime() << ";" << r->getCost() << ";" << transportTypeToString(r->getType()) << "\n";
            }
        }
        cout << "Routes saved to " << filename << endl;
    }
    
    string formatTxtOutput(const vector<shared_ptr<const Route>>& path) { /* ... same as before ... */
        if(path.empty())return "No path."; stringstream ss; double tT=0,tC=0,tD=0;
        ss<<"========================================\n"; ss<<"      R E C O M M E N D E D   R O U T E\n"; ss<<"========================================\n";
        ss<<"From: "<<path.front()->getSource()->getName()<<"\n"; ss<<"To:   "<<path.back()->getDestination()->getName()<<"\n\n"; ss<<"--- Steps ---\n";
        for(size_t i=0;i<path.size();++i){const auto& r=path[i]; ss<<"  "<<i+1<<". From "<<r->getSource()->getName()<<" to "<<r->getDestination()->getName()<<" by "<<transportTypeToString(r->getType())<<"\n     (Time: "<<r->getTime()<<"m, Cost: "<<r->getCost()<<"k, Dist: "<<r->getDistance()<<"m)\n";tT+=r->getTime();tC+=r->getCost();tD+=r->getDistance();}
        ss<<"\n--- Summary ---\n"; ss<<"  Total Time:     "<<tT<<" min\n"; ss<<"  Total Distance: "<<tD<<" m\n"; ss<<"  Total Cost:     Rp "<<fixed<<setprecision(3)<<(tC*1000)<<"\n"; ss<<"========================================\n"; return ss.str();
    }
    
    string formatCsvOutput(const vector<shared_ptr<const Route>>& path) {
        stringstream ss;
        ss << "start_location_id,start_location_name,end_location_id,end_location_name,transport_type,time_minutes,cost_k_rp,distance_meters\n";
        for (const auto& r : path) {
            ss << r->getSource()->getId() << "," << r->getSource()->getName() << "," << r->getDestination()->getId() << "," << r->getDestination()->getName()
               << "," << transportTypeToString(r->getType()) << "," << r->getTime() << "," << r->getCost() << "," << r->getDistance() << "\n";
        }
        return ss.str();
    }

    void saveOutput(const string& filename, const string& content, bool append = false) { /* ... same as before ... */
        ofstream file; if(append){file.open(filename,ios_base::app);}else{file.open(filename);} file<<content<<"\n\n";
        cout<<"Saved to "<<filename<<endl;
    }

    void listFiles(const string& type) {
        cout << "\n--- Available " << type << " Files ---\n";
        for (const auto& entry : filesystem::directory_iterator(".")) {
            string filename = entry.path().filename().string();
            if (filename.rfind(type, 0) == 0) {
                cout << "- " << filename << endl;
            }
        }
        cout << "---------------------------\n";
    }

    void deleteFile(const string& filename) { /* ... same as before ... */
        if(filesystem::remove(filename)){cout<<"Deleted "<<filename<<endl;}else{cerr<<"Error deleting "<<filename<<endl;}
    }

    string getNewOutputFilename(const string& base) {
        return base + "_" + to_string(findNextFileNumber(base+"_", ".txt")) + ".txt";
    }
};

void cli_addLocation(Graph& g) {
    string name; double lat, lon;
    cout << "Enter new location name: "; cin.ignore(); getline(cin, name);
    cout << "Enter latitude: "; cin >> lat;
    cout << "Enter longitude: "; cin >> lon;
    g.addLocation(name, lat, lon);
    cout << "Location '" << name << "' added successfully." << endl;
}

void cli_deleteLocation(Graph& g) {
    g.printAllLocations();
    cout << "Enter ID of location to delete: ";
    int id; cin >> id;
    if (g.deleteLocation(id)) {
        cout << "Location deleted successfully." << endl;
    } else {
        cout << "Invalid ID." << endl;
    }
}

void cli_updateLocation(Graph& g) {
    g.printAllLocations(); int id;
    cout << "Enter ID of location to update: "; cin >> id;
    if (!g.isValidLocation(id)) { cout << "Invalid ID.\n"; return; }
    string name; double lat, lon;
    cout << "Enter new name: "; cin.ignore(); getline(cin, name);
    cout << "Enter new latitude: "; cin >> lat;
    cout << "Enter new longitude: "; cin >> lon;
    if (g.updateLocation(id, name, lat, lon)) {
        cout << "Location updated successfully." << endl;
    }
}

void cli_addRoute(Graph& g) {
    g.printAllLocations();
    int srcId, destId;
    cout << "Enter source location ID: "; cin >> srcId;
    cout << "Enter destination location ID: "; cin >> destId;
    double dist, time, cost;
    cout << "Enter distance (meters): "; cin >> dist;
    cout << "Enter time (minutes): "; cin >> time;
    cout << "Enter cost (Thousand Rupiah): "; cin >> cost;
    cout << "Enter type (TRAIN, BUS, etc.): "; string typeStr; cin >> typeStr;
    g.addRoute(srcId, destId, dist, time, cost, stringToTransportType(typeStr));
    cout << "Route added." << endl;
}

void cli_deleteRoute(Graph& g) {
    g.printAllLocations();
    int srcId, destId;
    cout << "Enter source location ID for route to delete: "; cin >> srcId;
    cout << "Enter destination location ID for route to delete: "; cin >> destId;
    if (g.deleteRoute(srcId, destId)) {
        cout << "Route deleted successfully." << endl;
    } else {
        cout << "Route not found." << endl;
    }
}

void cli_showRecommendedPaths(const vector<vector<shared_ptr<const Route>>>& history, const FileManager& fm) {
    cout << "\n--- Recommended Path History ---\n";
    if (history.empty()) {
        cout << "No recommendations have been made yet in this session.\n";
    } else {
        for (size_t i = 0; i < history.size(); ++i) {
            cout << "--- History Item #" << i + 1 << " ---\n";
            cout << fm.formatTxtOutput(history[i]);
        }
    }
    cout << "--------------------------------\n";
}

// =================================================================================
// 4. MAIN PROGRAM FLOW
// =================================================================================

int main() {
    Graph transportationSystem;
    DecisionTree preferenceFinder;
    FileManager fileManager;

    vector<vector<shared_ptr<const Route>>> pathHistory;

    fileManager.loadAllData(transportationSystem);
    
    int choice = -1;
    while(choice != 0) {
        cout << "\n=============== MAIN MENU ===============\n"
             << "Locations & Routes:\n"
             << "  1. Show All Locations\n"
             << "  2. Show All Routes\n"
             << "  3. Add Location\n"
             << "  4. Update Location\n"
             << "  5. Delete Location\n"
             << "  6. Add Route\n"
             << "  7. Delete Route\n"
             << "\nPathfinding & Saving:\n"
             << "  8. Recommend a Path\n"
             << "  9. Show Recommended Path History\n"
             << "  10. Save LAST Recommended Path\n"
             << "  11. Save ALL Recommended Paths\n"
             << "\nFile Management:\n"
             << "  12. Save Current Graph Data to File\n"
             << "  13. Delete a File\n"
             << "\n0. Exit\n"
             << "=========================================\n"
             << "Enter your choice: ";
        cin >> choice;
        if (cin.fail()) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = -1;
        }

        switch(choice) {
            case 1: transportationSystem.printAllLocations(); break;
            case 2: transportationSystem.printAllRoutes(); break;
            case 3: cli_addLocation(transportationSystem); break;
            case 4: cli_updateLocation(transportationSystem); break;
            case 5: cli_deleteLocation(transportationSystem); break;
            case 6: cli_addRoute(transportationSystem); break;
            case 7: cli_deleteRoute(transportationSystem); break;
            case 8: {
                UserPreferences prefs = preferenceFinder.run();
                transportationSystem.printAllLocations();
                int startId=0, goalId=0;
                cout << "Enter Start ID: "; cin >> startId;
                cout << "Enter Goal ID: "; cin >> goalId;
                if(transportationSystem.isValidLocation(startId) && transportationSystem.isValidLocation(goalId)) {
                    vector<shared_ptr<const Route>> path = transportationSystem.findShortestPath(startId, goalId, prefs);
                    if (!path.empty()) {
                        pathHistory.push_back(path);
                        cout << fileManager.formatTxtOutput(path);
                    }
                } else {
                    cout << "Invalid location ID(s).\n";
                }
                break;
            }
            case 9: cli_showRecommendedPaths(pathHistory, fileManager); break;
            case 10: {
                if(pathHistory.empty()){cout<<"No path recommended yet.\n";}else{string f="output_last";fileManager.saveOutput(f+".txt",fileManager.formatTxtOutput(pathHistory.back()));fileManager.saveOutput(f+".csv",fileManager.formatCsvOutput(pathHistory.back()));}
                break;
            }
            case 11: {
                if(pathHistory.empty()){cout<<"No paths recommended yet.\n";}else{string f;cout<<"Base name for files: ";cin>>f;stringstream t,c;for(const auto&p:pathHistory){t<<fileManager.formatTxtOutput(p);c<<fileManager.formatCsvOutput(p);}fileManager.saveOutput(f+".txt",t.str());fileManager.saveOutput(f+".csv",c.str());}
                break;
            }
            case 12: {
                 fileManager.saveLocationsToCSV(transportationSystem, "input_locations_saved.csv");
                 fileManager.saveRoutesToCSV(transportationSystem, "input_routes_saved.csv");
                 break;
            }
            case 13: {
                string t;cout<<"Delete input or output file? (input/output): ";cin>>t;fileManager.listFiles(t);cout<<"Filename to delete: ";string fn;cin>>fn;fileManager.deleteFile(fn);
                break;
            }
            case 0: cout << "Exiting program. Goodbye!" << endl; break;
            default: cout << "Invalid choice. Please try again." << endl; break;
        }
    }

    } catch (const exception& e) {
        cerr << "A critical error occurred: " << e.what() << endl;
        return 1;
    }
    return 0;
}