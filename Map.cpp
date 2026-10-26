#include "Map.h"

#include <algorithm>
#include <fstream>
#include <sstream>

// ---------------------------- Helper Functions ---------------------------- //

/**
 * Removes spaces, tabs and line endings from both ends of a text.
 */
static std::string trim(const std::string& text) {
    const std::string whitespace = " \t\r\n";
    size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

/**
 * Tells whether a path ends with the .map extension.
 */
static bool hasMapExtension(const std::string& path) {
    const std::string extension = ".map";
    if (path.size() < extension.size()) {
        return false;
    }
    return path.compare(path.size() - extension.size(), extension.size(), extension) == 0;
}

// -------------------------------- Territory -------------------------------- //

// default constructor
Territory::Territory(const std::string& name, int id) :
    name(new std::string(name)),
    id(id),
    armies(0),
    owner(nullptr),
    neighbors(new std::vector<Territory*>()) {}

// copy constructor
Territory::Territory(const Territory& other) :
    name(new std::string(*other.name)),
    id(other.id),
    armies(other.armies),
    owner(other.owner),
    neighbors(new std::vector<Territory*>(*other.neighbors)) {}

// destructor
Territory::~Territory() {
    delete name;
    delete neighbors;
}

// assignment operator
Territory& Territory::operator=(const Territory& other) {
    if (this != &other) {
        *name = *other.name;
        id = other.id;
        armies = other.armies;
        owner = other.owner;
        *neighbors = *other.neighbors;
    }
    return *this;
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const Territory& territory) {
    out << *territory.name << " (armies: " << territory.armies << ", owner: ";
    if (territory.owner == nullptr) {
        out << "none";
    } else {
        out << territory.owner->getName();
    }
    out << ")";
    return out;
}

// returns the name of the territory
std::string Territory::getName() const {
    return *name;
}

// returns the id of the territory
int Territory::getId() const {
    return id;
}

// returns the number of armies on the territory
int Territory::getArmies() const {
    return armies;
}

// sets the number of armies on the territory
void Territory::setArmies(int armies) {
    this->armies = armies;
}

// returns the player who owns the territory (nullptr if nobody)
Player* Territory::getOwner() const {
    return owner;
}

// sets the player who owns the territory
void Territory::setOwner(Player* owner) {
    this->owner = owner;
}

// returns the list of territories adjacent to this one
std::vector<Territory*> Territory::getNeighbors() const {
    return *neighbors;
}

// adds an edge to another territory, ignoring duplicates and itself
void Territory::addNeighbor(Territory* territory) {
    if (territory != this && !isNeighborTo(territory)) {
        neighbors->push_back(territory);
    }
}

// tells whether the given territory is adjacent to this one
bool Territory::isNeighborTo(const Territory* territory) const {
    return std::find(neighbors->begin(), neighbors->end(), territory) != neighbors->end();
}

// removes all edges of this territory
void Territory::clearNeighbors() {
    neighbors->clear();
}

// -------------------------------- Continent -------------------------------- //

// default constructor
Continent::Continent(const std::string& name, int bonus) :
    name(new std::string(name)),
    bonus(bonus),
    territories(new std::vector<Territory*>()) {}

// copy constructor
Continent::Continent(const Continent& other) :
    name(new std::string(*other.name)),
    bonus(other.bonus),
    territories(new std::vector<Territory*>(*other.territories)) {}

// destructor
Continent::~Continent() {
    delete name;
    delete territories;
}

// assignment operator
Continent& Continent::operator=(const Continent& other) {
    if (this != &other) {
        *name = *other.name;
        bonus = other.bonus;
        *territories = *other.territories;
    }
    return *this;
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const Continent& continent) {
    out << "Continent " << *continent.name
        << " (bonus: " << continent.bonus
        << ", territories: " << continent.territories->size() << ")";
    return out;
}

// returns the name of the continent
std::string Continent::getName() const {
    return *name;
}

// returns the bonus given to a player who controls the whole continent
int Continent::getBonus() const {
    return bonus;
}

// returns the territories of the continent
std::vector<Territory*> Continent::getTerritories() const {
    return *territories;
}

// adds a territory to the continent
void Continent::addTerritory(Territory* territory) {
    territories->push_back(territory);
}

// tells whether the given territory is part of this continent
bool Continent::containsTerritory(const Territory* territory) const {
    return std::find(territories->begin(), territories->end(), territory) != territories->end();
}

// removes all territories of this continent
void Continent::clearTerritories() {
    territories->clear();
}

// ----------------------------------- Map ----------------------------------- //

// default constructor
Map::Map() :
    territories(new std::vector<Territory*>()),
    continents(new std::vector<Continent*>()) {}

// copy constructor
Map::Map(const Map& other) :
    territories(new std::vector<Territory*>()),
    continents(new std::vector<Continent*>()) {
    copyFrom(other);
}

// destructor
Map::~Map() {
    clear();
    delete territories;
    delete continents;
}

// assignment operator
Map& Map::operator=(const Map& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const Map& map) {
    out << "Map with " << map.territories->size() << " territories and "
        << map.continents->size() << " continents";
    for (Continent* continent : *map.continents) {
        out << "\n  " << *continent;
    }
    return out;
}

// deletes every territory and continent owned by the map
void Map::clear() {
    for (Territory* t : *territories) {
        delete t;
    }
    for (Continent* c : *continents) {
        delete c;
    }
    territories->clear();
    continents->clear();
}

// deep copy of another map: new territories, edges and continents
void Map::copyFrom(const Map& other) {
    // copy every territory, without its neighbors
    for (Territory* original : *other.territories) {
        Territory* copy = new Territory(*original);
        copy->clearNeighbors();
        territories->push_back(copy);
    }

    // rebuild the edges so the new territories are used instead
    for (size_t i = 0; i < other.territories->size(); i++) {
        for (Territory* neighbor : (*other.territories)[i]->getNeighbors()) {
            int index = other.indexOfTerritory(neighbor);
            if (index >= 0) {
                (*territories)[i]->addNeighbor((*territories)[index]);
            }
        }
    }

    // copy every continent using the new territories instead
    for (Continent* original : *other.continents) {
        Continent* copy = new Continent(*original);
        copy->clearTerritories();
        continents->push_back(copy);
        for (Territory* t : original->getTerritories()) {
            int index = other.indexOfTerritory(t);
            if (index >= 0) {
                copy->addTerritory((*territories)[index]);
            }
        }
    }
}

// returns the position of a territory in the territories list, or -1 if it is not there
int Map::indexOfTerritory(const Territory* territory) const {
    for (size_t i = 0; i < territories->size(); i++) {
        if ((*territories)[i] == territory) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// returns the number of territories in the map
int Map::getTerritoryCount() const {
    return static_cast<int>(territories->size());
}

// returns the number of continents in the map
int Map::getContinentCount() const {
    return static_cast<int>(continents->size());
}

// returns the territory with the given id, or nullptr if there is none
Territory* Map::getTerritoryById(int id) const {
    for (Territory* t : *territories) {
        if (t->getId() == id) {
            return t;
        }
    }
    return nullptr;
}

// returns the continent at the given 0-based position, or nullptr if out of range
Continent* Map::getContinentAt(int index) const {
    if (index < 0 || index >= getContinentCount()) {
        return nullptr;
    }
    return (*continents)[index];
}

// adds a territory to the map
void Map::addTerritory(Territory* territory) {
    territories->push_back(territory);
}

// adds a continent to the map
void Map::addContinent(Continent* continent) {
    continents->push_back(continent);
}

// DFS algorithm to count the number of allowed territories reachable from a starting territory
size_t Map::countReachableTerritories(Territory* start, const std::vector<Territory*>& allowed) const {
    std::vector<Territory*> visited;
    std::vector<Territory*> toVisit;
    visited.push_back(start);
    toVisit.push_back(start);
 
    while (!toVisit.empty()) {
        Territory* current = toVisit.back();
        toVisit.pop_back();
 
        for (Territory* neighbor : current->getNeighbors()) {
            bool isAllowed = std::find(allowed.begin(), allowed.end(), neighbor) != allowed.end();
            bool alreadyVisited = std::find(visited.begin(), visited.end(), neighbor) != visited.end();
            if (isAllowed && !alreadyVisited) {
                visited.push_back(neighbor);
                toVisit.push_back(neighbor);
            }
        }
    }
    return visited.size();
}

// check 1: every territory can be reached from the first one
bool Map::isMapConnected() const {
    if (territories->empty()) {
        return true;
    }
    return countReachableTerritories((*territories)[0], *territories) == territories->size();
}

// check 2: every territory can be reached from the first one in its continent
bool Map::isEachContinentConnected() const {
    for (Continent* continent : *continents) {
        std::vector<Territory*> territories = continent->getTerritories();
        if (territories.empty()) {
            continue;
        }
        if (countReachableTerritories(territories[0], territories) != territories.size()) {
            return false;
        }
    }
    return true;
}

// check 3: every territory is listed in exactly one continent
bool Map::isEachTerritorySolelyOwned() const {
    for (Territory* t : *territories) {
        int count = 0;
        for (Continent* continent : *continents) {
            if (continent->containsTerritory(t)) {
                count++;
            }
        }
        if (count != 1) {
            return false;
        }
    }
    return true;
}

// validates the map and prints the results of the three checks if requested
bool Map::validate(bool printResults) const {
    bool mapConnected = isMapConnected();
    bool continentsConnected = isEachContinentConnected();
    bool solelyOwned = isEachTerritorySolelyOwned();

    if (printResults) {
        std::cout << "  [1] Map is a connected graph:           " << (mapConnected ? "PASS" : "FAIL") << std::endl;
        std::cout << "  [2] Continents are connected subgraphs: " << (continentsConnected ? "PASS" : "FAIL") << std::endl;
        std::cout << "  [3] Each territory in one continent:    " << (solelyOwned ? "PASS" : "FAIL") << std::endl;
    }

    return mapConnected && continentsConnected && solelyOwned;
}

// -------------------------------- MapLoader -------------------------------- //

// default constructor
MapLoader::MapLoader() :
    lastError(new std::string("")) {}

// copy constructor
MapLoader::MapLoader(const MapLoader& other) :
    lastError(new std::string(*other.lastError)) {}

// destructor
MapLoader::~MapLoader() {
    delete lastError;
}

// assignment operator
MapLoader& MapLoader::operator=(const MapLoader& other) {
    if (this != &other) {
        *lastError = *other.lastError;
    }
    return *this;
}

// stream insertion operator
std::ostream& operator<<(std::ostream& out, const MapLoader& loader) {
    out << "MapLoader (last error: ";
    if (loader.lastError->empty()) {
        out << "none";
    } else {
        out << *loader.lastError;
    }
    out << ")";
    return out;
}

// returns the reason why the last load failed
std::string MapLoader::getLastError() const {
    return *lastError;
}

// stores the error message, deletes the half-built map and returns nullptr
Map* MapLoader::reject(Map* map, const std::string& reason) {
    *lastError = reason;
    delete map;
    return nullptr;
}

// error messages for MapLoader
const std::string FILE_NOT_FOUND = "File not found";
const std::string NOT_A_MAP_FILE = "Not a .map file";
const std::string INVALID_MAP_CONFIGURATION = "Invalid map configuration";
const std::string UNKNOWN_ERROR = "Unknown error";

// loads a .map file into a Map object
Map* MapLoader::loadMap(const std::string& path) {
    *lastError = "";
    std::ifstream file(path);
    if (!file.is_open()) {
        return reject(nullptr, FILE_NOT_FOUND);
    }
 
    if (!hasMapExtension(path)) {
        return reject(nullptr, NOT_A_MAP_FILE);
    }
 
    Map* map = new Map();
    std::string section = "";
    std::string line;
 
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            section = line;
            continue;
        }
 
        std::istringstream in(line);
 
        if (section == "[continents]") {
            // line format: name bonus color
            std::string name;
            int bonus;
            if (!(in >> name >> bonus)) {
                return reject(map, INVALID_MAP_CONFIGURATION);
            }
            map->addContinent(new Continent(name, bonus));
        } else if (section == "[countries]") {
            // line format: id name continentNumber x y
            int id;
            std::string name;
            int continentNumber;
            if (!(in >> id >> name >> continentNumber)) {
                return reject(map, INVALID_MAP_CONFIGURATION);
            }
            Continent* continent = map->getContinentAt(continentNumber - 1);
            if (id != map->getTerritoryCount() + 1 || continent == nullptr) {
                return reject(map, INVALID_MAP_CONFIGURATION);
            }
            Territory* territory = new Territory(name, id);
            map->addTerritory(territory);
            continent->addTerritory(territory);
        } else if (section == "[borders]") {
            // line format: id neighbor1 neighbor2 ...
            int id;
            if (!(in >> id)) {
                return reject(map, INVALID_MAP_CONFIGURATION);
            }
            Territory* from = map->getTerritoryById(id);
            if (from == nullptr) {
                return reject(map, INVALID_MAP_CONFIGURATION);
            }
            int neighborId;
            while (in >> neighborId) {
                Territory* to = map->getTerritoryById(neighborId);
                if (to == nullptr) {
                    return reject(map, INVALID_MAP_CONFIGURATION);
                }
                from->addNeighbor(to);
                to->addNeighbor(from);
            }
            if (!in.eof()) {
                return reject(map, INVALID_MAP_CONFIGURATION);
            }
        }
    }
 
    if (file.bad()) {
        return reject(map, UNKNOWN_ERROR);
    }
 
    if (map->getContinentCount() == 0 || map->getTerritoryCount() == 0) {
        return reject(map, INVALID_MAP_CONFIGURATION);
    }

    return map;
}