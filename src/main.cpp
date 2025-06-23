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
#include <filesystem> // Required for file system operations

// Use standard namespace for cleaner code
using namespace std;

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =================================================================================
// 0. CLI STYLING (ASCII ART & COLORS)
// =================================================================================

namespace Color {
    const string RED = "\033[1;31m";
    const string GREEN = "\033[1;32m";
    const string YELLOW = "\033[1;33m";
    const string BLUE = "\033[1;34m";
    const string MAGENTA = "\033[1;35m";
    const string CYAN = "\033[1;36m";
    const string WHITE = "\033[1;37m";
    const string RESET = "\033[0m";
}

void cli_printHeader() {
    cout << Color::CYAN << R"(
 __  __     _     ____   _____ 
|  \/  |   / \   |  _ \ / ____|
| |\/| |  / _ \  | |_) | (___  
| |  | | / ___ \ |  __/ \___ \ 
|_|  |_|/_/   \_\|_|    |____/ 
                               
)" << Color::RESET;
    cout << Color::CYAN << "\n================= M A P S =================" << Color::RESET << endl;
    cout << Color::WHITE << "      Multi-Attribute Pathing System" << Color::RESET << endl;
}


// =================================================================================
// 1. FORWARD DECLARATIONS & CORE DATA TYPES
// =================================================================================
class Location;
class Route;
class Graph;
struct UserPreferences;
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
    string upper_s = s;
    transform(upper_s.begin(), upper_s.end(), upper_s.begin(), ::toupper);
    if (upper_s == "TRAIN") return TransportationType::TRAIN;
    if (upper_s == "BUS") return TransportationType::BUS;
    if (upper_s == "BOAT") return TransportationType::BOAT;
    if (upper_s == "PLANE") return TransportationType::PLANE;
    return TransportationType::ANY;
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
    string question;
    bool isLeaf = false;
    UserPreferences preferences;
    map<string, shared_ptr<TreeNode>> children;
    TreeNode(string qu) : question(move(qu)) {}
    TreeNode(UserPreferences pr) : isLeaf(true), preferences(move(pr)) {}
};

class DecisionTree {
private:
    shared_ptr<TreeNode> root;

    void buildTree() {
        auto budget = UserPreferences{"Budget Traveler", 1.0, 10.0, 3.0};
        auto balanced = UserPreferences{"Balanced", 5.0, 5.0, 5.0};
        auto business = UserPreferences{"Business", 10.0, 2.0, 1.0};
        root = make_shared<TreeNode>("What is your main priority?");
        root->children["1. Fastest"] = make_shared<TreeNode>(business);
        root->children["2. Cheapest"] = make_shared<TreeNode>(budget);
        root->children["3. Balanced"] = make_shared<TreeNode>(balanced);
    }

    void printTree(const shared_ptr<TreeNode>& n, int i) const {
        if (!n) return;
        for (int k = 0; k < i; ++k) cout << "  ";
        if (n->isLeaf) {
            cout << "-> LEAF: " << n->preferences.profileName << "\n";
        } else {
            cout << "Q: " << n->question << "\n";
            for (const auto& p : n->children) {
                for (int k = 0; k < i; ++k) cout << "  ";
                cout << "  [" << p.first << "]\n";
                printTree(p.second, i + 2);
            }
        }
    }

public:
    DecisionTree() {
        buildTree();
    }

    UserPreferences run() const {
        cout << Color::YELLOW << "\n--- Determining Your Travel Style ---" << Color::RESET;
        shared_ptr<const TreeNode> currentNode = root;
        string answer;
        while (!currentNode->isLeaf) {
            cout << "\nQ: " << currentNode->question << "\n";
            for (const auto& p : currentNode->children) {
                cout << "   " << p.first << "\n";
            }
            cout << "Your choice: ";
            cin >> answer;
            string matchedKey;
            for (const auto& p : currentNode->children) {
                if (p.first.rfind(answer, 0) == 0) {
                    matchedKey = p.first;
                    break;
                }
            }
            if (!matchedKey.empty()) {
                currentNode = currentNode->children.at(matchedKey);
            } else {
                cout << Color::RED << "Invalid." << Color::RESET << "\n";
            }
        }
        UserPreferences finalPrefs = currentNode->preferences;
        cout << Color::GREEN << "\nProfile set to: " << finalPrefs.profileName << Color::RESET << "\n";
        map<string, TransportationType> transportChoices = {
            {"1. Train", TransportationType::TRAIN},
            {"2. Bus", TransportationType::BUS},
            {"3. Plane", TransportationType::PLANE},
            {"4. Boat", TransportationType::BOAT},
            {"5. Any", TransportationType::ANY}
        };
        bool choiceMade = false;
        while (!choiceMade) {
            cout << "\nWhich mode of transport do you prefer?\n";
            for (const auto& p : transportChoices) {
                cout << "   " << p.first << endl;
            }
            cout << "Your choice: ";
            cin >> answer;
            string matchedKey;
            for (const auto& p : transportChoices) {
                if (p.first.rfind(answer, 0) == 0) {
                    matchedKey = p.first;
                    break;
                }
            }
            if (!matchedKey.empty()) {
                finalPrefs.preferredTransport = transportChoices.at(matchedKey);
                choiceMade = true;
            } else {
                cout << Color::RED << "Invalid." << Color::RESET << "\n";
            }
        }
        cout << Color::GREEN << "Transportation preference: " << transportTypeToString(finalPrefs.preferredTransport) << Color::RESET << endl;
        return finalPrefs;
    }

    void visualize() const {
        cout << "\n--- Decision Tree ---\n";
        printTree(root, 0);
        cout << "-------------------\n";
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
    Location(int id = 0, string nm = "N/A", double lat = 0, double lon = 0) : id(id), name(nm), latitude(lat), longitude(lon) {}
    int getId() const { return id; }
    string getName() const { return name; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }
    void update(string nm, double lat, double lon) { name = nm; latitude = lat; longitude = lon; }
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
    Route(Location* sr, Location* de, double dis, double tim, double cos, TransportationType t) : source(sr), destination(de), distance(dis), time(tim), cost(cos), type(t) {}
    virtual ~Route() = default;
    virtual double calculateWeight(const UserPreferences& p, const GraphStats& s) const = 0;
    Location* getSource() const { return source; }
    Location* getDestination() const { return destination; }
    double getDistance() const { return distance; }
    double getTime() const { return time; }
    double getCost() const { return cost; }
    TransportationType getType() const { return type; }
};

class ConcreteRoute : public Route {
public:
    ConcreteRoute(Location* s, Location* d, double di, double ti, double co, TransportationType ty) : Route(s, d, di, ti, co, ty) {}
    double calculateWeight(const UserPreferences& p, const GraphStats& s) const override {
        double nt = time / s.maxTime;
        double nc = cost / s.maxCost;
        double nd = distance / s.maxDistance;
        double w = (nt * p.timeWeight) + (nc * p.costWeight) + (nd * p.distanceWeight);
        if (p.preferredTransport != TransportationType::ANY && this->type != p.preferredTransport) {
            w += 1000.0;
        }
        return w;
    }
};

class Graph {
private:
    map<int, Location> locations;
    map<int, vector<shared_ptr<Route>>> adjList;
    GraphStats stats;
    int nextId = 1;

    double heuristic(const Location& a, const Location& b) const {
        const double R = 6371;
        double lat1 = a.getLatitude() * M_PI / 180.0;
        double lon1 = a.getLongitude() * M_PI / 180.0;
        double lat2 = b.getLatitude() * M_PI / 180.0;
        double lon2 = b.getLongitude() * M_PI / 180.0;
        double dlon = lon2 - lon1;
        double dlat = lat2 - lat1;
        double val = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);
        double c = 2 * asin(sqrt(val));
        return R * c;
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
    Graph() = default;

    const map<int, Location>& getAllLocations() const { return locations; }
    const map<int, vector<shared_ptr<Route>>>& getAdjList() const { return adjList; }

    int addLocation(const string& n, double la, double lo) {
        int i = nextId++;
        locations[i] = Location(i, n, la, lo);
        adjList[i] = {};
        return i;
    }

    void addRoute(int s, int d, double di, double ti, double co, TransportationType ty) {
        if (!locations.count(s) || !locations.count(d)) return;
        if (ty == TransportationType::ANY) {
            throw runtime_error("Route must have a specific type.");
        }
        adjList.at(s).push_back(make_shared<ConcreteRoute>(&locations.at(s), &locations.at(d), di, ti, co, ty));
        if (ti > stats.maxTime) stats.maxTime = ti;
        if (co > stats.maxCost) stats.maxCost = co;
        if (di > stats.maxDistance) stats.maxDistance = di;
    }

    bool updateLocation(int i, const string& n, double la, double lo) {
        if (!locations.count(i)) return false;
        locations.at(i).update(n, la, lo);
        return true;
    }

    bool deleteRoute(int s, int d) {
        if (!adjList.count(s)) return false;
        auto& rs = adjList.at(s);
        auto oS = rs.size();
        rs.erase(remove_if(rs.begin(), rs.end(), [d](const shared_ptr<Route>& r) {
            return r->getDestination()->getId() == d;
        }), rs.end());
        return rs.size() < oS;
    }

    bool deleteLocation(int i) {
        if (!locations.count(i)) return false;
        locations.erase(i);
        adjList.erase(i);
        for (auto& p : adjList) {
            auto& rs = p.second;
            rs.erase(remove_if(rs.begin(), rs.end(), [i](const shared_ptr<Route>& r) {
                return r->getDestination()->getId() == i;
            }), rs.end());
        }
        return true;
    }

    void printAllLocations() const {
        cout << "\n--- All Locations ---\n";
        if (locations.empty()) {
            cout << "No locations loaded.\n";
        } else {
            for (const auto& p : locations) {
                cout << "ID: " << p.first << "\t-> " << p.second.getName() << endl;
            }
        }
        cout << "---------------------\n";
    }

    // ##############################################################################
    // #
    // #   THE FIX FOR SHOWING ROUTE IDS IS HERE
    // #
    // #   This function now prints the source and destination IDs for clarity.
    // #
    void printAllRoutes() const {
        cout << "\n--- All Loaded Routes ---\n";
        bool routesExist = false;
        for (const auto& pair : adjList) {
            if (!pair.second.empty()) {
                routesExist = true;
                cout << "FROM: " << pair.first << " (" << locations.at(pair.first).getName() << ")" << endl;
                for (const auto& r : pair.second) {
                    cout << "  -> TO: " << left << setw(4) << r->getDestination()->getId()
                         << left << setw(15) << ("(" + r->getDestination()->getName() + ")")
                         << " | BY: " << left << setw(7) << transportTypeToString(r->getType())
                         << " | Time: " << r->getTime() << "m"
                         << " | Cost: " << r->getCost() << "k"
                         << " | Dist: " << r->getDistance() << "m" << endl;
                }
            }
        }
        if (!routesExist) {
            cout << "No routes loaded.\n";
        }
        cout << "------------------------\n";
    }
    // #
    // ##############################################################################

    bool isValidLocation(int i) const {
        return locations.count(i);
    }

    vector<shared_ptr<const Route>> findShortestPath(int sI, int gI, const UserPreferences& pr) const {
        using QE = pair<double, int>;
        priority_queue<QE, vector<QE>, greater<QE>> o;
        map<int, int> cf;
        map<int, double> gs;
        for (const auto& p : locations) {
            gs[p.first] = numeric_limits<double>::infinity();
        }
        gs[sI] = 0;
        o.push({heuristic(locations.at(sI), locations.at(gI)), sI});
        cout << "\nSearching...\n";
        while (!o.empty()) {
            int cI = o.top().second;
            o.pop();
            if (cI == gI) {
                return reconstructPath(cf, cI);
            }
            if (adjList.count(cI)) {
                for (const auto& r : adjList.at(cI)) {
                    int nI = r->getDestination()->getId();
                    double tgs = gs.at(cI) + r->calculateWeight(pr, stats);
                    if (tgs < gs.at(nI)) {
                        cf[nI] = cI;
                        gs[nI] = tgs;
                        double fs = gs.at(nI) + heuristic(locations.at(nI), locations.at(gI));
                        o.push({fs, nI});
                    }
                }
            }
        }
        cout << Color::RED << "No path found from " << locations.at(sI).getName() << " to " << locations.at(gI).getName() << Color::RESET << ".\n";
        return {};
    }
};

// =================================================================================
// 4. FILE MANAGER CLASS
// =================================================================================
class FileManager {
private:
    const filesystem::path inputDir = "../input";
    const filesystem::path locationsDir = inputDir / "locations";
    const filesystem::path routesDir = inputDir / "routes";
    const filesystem::path outputDir = "../output";
    const filesystem::path txtDir = outputDir / "txt";
    const filesystem::path csvDir = outputDir / "csv";

    void ensureDirectoriesExist() const {
        filesystem::create_directories(locationsDir);
        filesystem::create_directories(routesDir);
        filesystem::create_directories(txtDir);
        filesystem::create_directories(csvDir);
    }

public:
    FileManager() {
        ensureDirectoriesExist();
    }

    void loadAllData(Graph& g) const {
        cout << "\n--- Loading All Input Data ---\n";
        if (filesystem::exists(locationsDir)) {
            for (const auto& entry : filesystem::directory_iterator(locationsDir)) {
                string path = entry.path().string();
                ifstream file(path);
                if (!file.is_open()) continue;
                string line;
                getline(file, line); // Skip header
                while (getline(file, line)) {
                    stringstream ss(line);
                    string id, name, lat, lon;
                    getline(ss, id, ';'); getline(ss, name, ';'); getline(ss, lat, ';'); getline(ss, lon, ';');
                    if (!name.empty()) { g.addLocation(name, stod(lat), stod(lon)); }
                }
                cout << "Loaded locations from " << path << endl;
            }
        }
        if (filesystem::exists(routesDir)) {
            for (const auto& entry : filesystem::directory_iterator(routesDir)) {
                string path = entry.path().string();
                ifstream file(path);
                if (!file.is_open()) continue;
                string line;
                getline(file, line); // Skip header
                while (getline(file, line)) {
                    stringstream ss(line);
                    string src, dest, dist, time, cost, type;
                    getline(ss, src, ';'); getline(ss, dest, ';'); getline(ss, dist, ';'); getline(ss, time, ';'); getline(ss, cost, ';'); getline(ss, type, ';');
                    if (!src.empty()) { g.addRoute(stoi(src), stoi(dest), stod(dist), stod(time), stod(cost), stringToTransportType(type));}
                }
                cout << "Loaded routes from " << path << endl;
            }
        }
        cout << "----------------------------\n";
    }

    void saveLocationsToCSV(const Graph& g, const string& f) const {
        ofstream file(locationsDir / f);
        file << "id;name;latitude;longitude\n";
        for (const auto& [id, loc] : g.getAllLocations()) {
            file << id << ";" << loc.getName() << ";" << fixed << setprecision(4) << loc.getLatitude() << ";" << loc.getLongitude() << "\n";
        }
        cout << Color::GREEN << "Locations saved to " << (locationsDir / f).string() << Color::RESET << endl;
    }

    void saveRoutesToCSV(const Graph& g, const string& f) const {
        ofstream file(routesDir / f);
        file << "source_id;dest_id;distance_m;time_min;cost_k_rp;type\n";
        for (const auto& [fromId, routes] : g.getAdjList()) {
            for (const auto& r : routes) {
                file << fromId << ";" << r->getDestination()->getId() << ";" << r->getDistance() << ";" << r->getTime() << ";" << r->getCost() << ";" << transportTypeToString(r->getType()) << "\n";
            }
        }
        cout << Color::GREEN << "Routes saved to " << (routesDir / f).string() << Color::RESET << endl;
    }
    
    // This version is for the colorful console output
    string formatTxtOutputForConsole(const vector<shared_ptr<const Route>>& p) const {
        if (p.empty()) return "No path.";
        stringstream s;
        double tT = 0, tC = 0, tD = 0;
        s << Color::CYAN << "========================================\n";
        s << "      R E C O M M E N D E D   R O U T E\n";
        s << "========================================\n" << Color::RESET;
        s << Color::WHITE << "From: " << p.front()->getSource()->getName() << "\n" << "To:   " << p.back()->getDestination()->getName() << "\n\n" << Color::YELLOW << "--- Steps ---\n" << Color::RESET;
        for (size_t i = 0; i < p.size(); ++i) {
            const auto& r = p[i];
            s << "  " << i + 1 << ". From " << r->getSource()->getName() << " to " << r->getDestination()->getName() << " by " << transportTypeToString(r->getType()) << "\n     (Time: " << r->getTime() << "m, Cost: " << r->getCost() << "k, Dist: " << r->getDistance() << "m)\n";
            tT += r->getTime(); tC += r->getCost(); tD += r->getDistance();
        }
        s << Color::YELLOW << "\n--- Summary ---\n" << Color::RESET;
        s << "  Total Time:     " << tT << " min\n";
        s << "  Total Distance: " << tD << " m\n";
        s << "  Total Cost:     Rp " << fixed << setprecision(3) << (tC * 1000) << "\n";
        s << Color::CYAN << "========================================\n" << Color::RESET;
        return s.str();
    }
    
    // ##############################################################################
    // #
    // #   THE FIX FOR THE FILE OUTPUT IS HERE
    // #
    // #   This new function creates a plain text string with NO color codes,
    // #   making it suitable for saving to a .txt file.
    // #
    string formatTxtOutputForFile(const vector<shared_ptr<const Route>>& p) const {
        if (p.empty()) return "No path.";
        stringstream s;
        double tT = 0, tC = 0, tD = 0;
        s << "========================================\n";
        s << "      R E C O M M E N D E D   R O U T E\n";
        s << "========================================\n";
        s << "From: " << p.front()->getSource()->getName() << "\n" << "To:   " << p.back()->getDestination()->getName() << "\n\n" << "--- Steps ---\n";
        for (size_t i = 0; i < p.size(); ++i) {
            const auto& r = p[i];
            s << "  " << i + 1 << ". From " << r->getSource()->getName() << " to " << r->getDestination()->getName() << " by " << transportTypeToString(r->getType()) << "\n     (Time: " << r->getTime() << "m, Cost: " << r->getCost() << "k, Dist: " << r->getDistance() << "m)\n";
            tT += r->getTime(); tC += r->getCost(); tD += r->getDistance();
        }
        s << "\n--- Summary ---\n";
        s << "  Total Time:     " << tT << " min\n";
        s << "  Total Distance: " << tD << " m\n";
        s << "  Total Cost:     Rp " << fixed << setprecision(3) << (tC * 1000) << "\n";
        s << "========================================\n";
        return s.str();
    }
    // #
    // ##############################################################################

    string formatCsvOutput(const vector<shared_ptr<const Route>>& p) const {
        stringstream s;
        s << "start_id,start_name,end_id,end_name,type,time,cost,dist\n";
        for (const auto& r : p) {
            s << r->getSource()->getId() << "," << r->getSource()->getName() << "," << r->getDestination()->getId() << "," << r->getDestination()->getName() << "," << transportTypeToString(r->getType()) << "," << r->getTime() << "," << r->getCost() << "," << r->getDistance() << "\n";
        }
        return s.str();
    }

    void saveOutput(const string& b, const string& t, const string& c, bool a = false) const {
        filesystem::path tf = txtDir / (b + ".txt");
        filesystem::path cf = csvDir / (b + ".csv");
        ofstream tO, cO;
        if (a) {
            tO.open(tf, ios_base::app);
            cO.open(cf, ios_base::app);
        } else {
            tO.open(tf);
            cO.open(cf);
        }
        tO << t << "\n";
        cO << c;
        cout << Color::GREEN << "Saved output to " << tf.string() << " and " << cf.string() << Color::RESET << endl;
    }

    void listFiles(const string& t) const {
        filesystem::path d;
        if (t == "input_locations") d = locationsDir;
        else if (t == "input_routes") d = routesDir;
        else if (t == "output_txt") d = txtDir;
        else if (t == "output_csv") d = csvDir;
        else {
            cout << Color::RED << "Invalid type." << Color::RESET << endl;
            return;
        }
        cout << "\n--- Files in " << d.string() << " ---\n";
        for (const auto& e : filesystem::directory_iterator(d)) {
            cout << "- " << e.path().filename().string() << endl;
        }
        cout << "-----------------------\n";
    }

    void deleteFile() const {
        string t, f;
        cout << "Type to delete? (input_locations, input_routes, output_txt, output_csv): ";
        cin >> t;
        listFiles(t);
        cout << "Filename to delete: ";
        cin >> f;
        filesystem::path p;
        if (t == "input_locations") p = locationsDir / f;
        else if (t == "input_routes") p = routesDir / f;
        else if (t == "output_txt") p = txtDir / f;
        else if (t == "output_csv") p = csvDir / f;
        else {
            cout << Color::RED << "Invalid type." << Color::RESET << endl;
            return;
        }
        if (filesystem::remove(p)) {
            cout << Color::GREEN << "Deleted " << p.string() << Color::RESET << endl;
        } else {
            cerr << Color::RED << "Error deleting " << p.string() << Color::RESET << endl;
        }
    }
};

// =================================================================================
// 5. CLI HELPER FUNCTIONS
// =================================================================================
void cli_addLocation(Graph& g) { /* ... same as before ... */
    string n; double la, lo; cout << "Name: "; cin.ignore(); getline(cin, n); cout << "Lat: "; cin >> la; cout << "Lon: "; cin >> lo;
    g.addLocation(n, la, lo); cout << Color::GREEN << "Added." << Color::RESET << "\n";
}
void cli_deleteLocation(Graph& g) { /* ... same as before ... */
    g.printAllLocations(); cout << "ID to delete: "; int i; cin >> i;
    if (g.deleteLocation(i)) { cout << Color::GREEN << "Deleted." << Color::RESET << "\n"; }
    else { cout << Color::RED << "Invalid ID." << Color::RESET << "\n"; }
}
void cli_updateLocation(Graph& g) { /* ... same as before ... */
    g.printAllLocations(); int i; cout << "ID to update: "; cin >> i; if (!g.isValidLocation(i)) { cout << Color::RED << "Invalid ID.\n" << Color::RESET; return; }
    string n; double la, lo; cout << "New name: "; cin.ignore(); getline(cin, n); cout << "New lat: "; cin >> la; cout << "New lon: "; cin >> lo;
    if (g.updateLocation(i, n, la, lo)) { cout << Color::GREEN << "Updated." << Color::RESET << "\n"; }
}
void cli_addRoute(Graph& g) { /* ... same as before ... */
    g.printAllLocations(); int s, d; cout << "Src ID: "; cin >> s; cout << "Dest ID: "; cin >> d;
    double di, ti, c; cout << "Dist (m): "; cin >> di; cout << "Time (m): "; cin >> ti; cout << "Cost (kRp): "; cin >> c;
    cout << "Type: "; string ts; cin >> ts; g.addRoute(s, d, di, ti, c, stringToTransportType(ts));
    cout << Color::GREEN << "Route added." << Color::RESET << "\n";
}
void cli_deleteRoute(Graph& g) { /* ... same as before ... */
    g.printAllLocations(); int s, d; cout << "Src ID: "; cin >> s; cout << "Dest ID: "; cin >> d;
    if (g.deleteRoute(s, d)) { cout << Color::GREEN << "Route deleted." << Color::RESET << "\n"; }
    else { cout << Color::RED << "Not found." << Color::RESET << "\n"; }
}
void cli_showRecommendedPaths(const vector<vector<shared_ptr<const Route>>>& h, const FileManager& f) {
    cout << "\n--- Recommended Path History ---\n";
    if (h.empty()) {
        cout << "No recommendations in this session.\n";
    } else {
        for (size_t i = 0; i < h.size(); ++i) {
            cout << "--- History Item #" << i + 1 << " ---\n";
            cout << f.formatTxtOutputForConsole(h[i]);
        }
    }
    cout << "--------------------------------\n";
}

void cli_printMenu() { /* ... same as before ... */
    cout << Color::YELLOW << "\n=============== MAIN MENU ===============\n" << Color::RESET
         << Color::WHITE << "Locations & Routes:\n" << Color::RESET
         << Color::GREEN << "  1. " << Color::WHITE << "Show All Locations\n"
         << Color::GREEN << "  2. " << Color::WHITE << "Show All Routes\n"
         << Color::GREEN << "  3. " << Color::WHITE << "Add Location\n"
         << Color::GREEN << "  4. " << Color::WHITE << "Update Location\n"
         << Color::GREEN << "  5. " << Color::WHITE << "Delete Location\n"
         << Color::GREEN << "  6. " << Color::WHITE << "Add Route\n"
         << Color::GREEN << "  7. " << Color::WHITE << "Delete Route\n"
         << Color::WHITE << "\nPathfinding & Saving:\n" << Color::RESET
         << Color::GREEN << "  8. " << Color::WHITE << "Recommend a Path\n"
         << Color::GREEN << "  9. " << Color::WHITE << "Show Recommended Path History\n"
         << Color::GREEN << "  10. " << Color::WHITE << "Save LAST Recommended Path\n"
         << Color::GREEN << "  11. " << Color::WHITE << "Save ALL Recommended Paths\n"
         << Color::WHITE << "\nFile Management:\n" << Color::RESET
         << Color::GREEN << "  12. " << Color::WHITE << "Save Current Graph Data to File\n"
         << Color::GREEN << "  13. " << Color::WHITE << "Delete a File\n"
         << Color::RED << "\n  0. Exit\n" << Color::RESET
         << Color::YELLOW << "=========================================\n" << Color::RESET
         << Color::MAGENTA << "Enter your choice: " << Color::RESET;
}

// =================================================================================
// 6. MAIN PROGRAM FLOW (CLI)
// =================================================================================
int main() {
    FileManager fileManager;
    Graph transportationSystem;
    DecisionTree preferenceFinder;
    vector<vector<shared_ptr<const Route>>> pathHistory;

    cli_printHeader();

    try {
        fileManager.loadAllData(transportationSystem);

        int choice = -1;
        while (choice != 0) {
            cli_printMenu();
            cin >> choice;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                choice = -1;
            }

            switch (choice) {
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
                    int startId = 0, goalId = 0;
                    cout << "Enter Start ID: "; cin >> startId;
                    cout << "Enter Goal ID: "; cin >> goalId;
                    if (transportationSystem.isValidLocation(startId) && transportationSystem.isValidLocation(goalId)) {
                        vector<shared_ptr<const Route>> path = transportationSystem.findShortestPath(startId, goalId, prefs);
                        if (!path.empty()) {
                            pathHistory.push_back(path);
                            cout << fileManager.formatTxtOutputForConsole(path);
                        }
                    } else {
                        cout << Color::RED << "Invalid location ID(s).\n" << Color::RESET;
                    }
                    break;
                }
                case 9: cli_showRecommendedPaths(pathHistory, fileManager); break;
                case 10: {
                    if (pathHistory.empty()) {
                        cout << Color::YELLOW << "No path recommended yet.\n" << Color::RESET;
                    } else {
                        string f = "output_last";
                        // Using the correct formatter for each file type
                        fileManager.saveOutput(f, fileManager.formatTxtOutputForFile(pathHistory.back()), fileManager.formatCsvOutput(pathHistory.back()));
                    }
                    break;
                }
                case 11: {
                    if (pathHistory.empty()) {
                        cout << Color::YELLOW << "No paths recommended yet.\n" << Color::RESET;
                    } else {
                        string f;
                        cout << "Base name for files: ";
                        cin >> f;
                        stringstream t, c;
                        for (const auto& p : pathHistory) {
                            t << fileManager.formatTxtOutputForFile(p); // Use file-safe formatter
                            c << fileManager.formatCsvOutput(p);
                        }
                        fileManager.saveOutput(f, t.str(), c.str());
                    }
                    break;
                }
                case 12: {
                    fileManager.saveLocationsToCSV(transportationSystem, "input_locations_saved.csv");
                    fileManager.saveRoutesToCSV(transportationSystem, "input_routes_saved.csv");
                    break;
                }
                case 13:
                    fileManager.deleteFile();
                    break;
                case 0:
                    cout << Color::CYAN << "Exiting program. Goodbye!" << Color::RESET << endl;
                    break;
                default:
                    cout << Color::RED << "Invalid choice. Please try again." << Color::RESET << endl;
                    break;
            }
        }

    } catch (const exception& e) {
        cerr << Color::RED << "A critical error occurred: " << e.what() << Color::RESET << endl;
        return 1;
    }
    return 0;
}
