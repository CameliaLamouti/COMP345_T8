#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <string>
#include <vector>

// TO FIX once Player class is implemented
// #include "Player.h"
class Player;

/**
 * Territory
 * A node in the map graph.
 */
class Territory {
private:
    std::string* name;
    int id;
    int armies;
    Player* owner;
    std::vector<Territory*>* neighbors;

public:
    // default constructor
    Territory(const std::string& name, int id);
    // copy constructor
    Territory(const Territory& other);
    // destructor
    ~Territory();
    // assignment operator
    Territory& operator=(const Territory& other);
    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Territory& territory);

    // getters/setters
    std::string getName() const;
    int getId() const;
    int getArmies() const;
    void setArmies(int armies);
    Player* getOwner() const;
    void setOwner(Player* owner);
    std::vector<Territory*> getNeighbors() const;

    // adds an edge to another territory (ignores duplicates and itself)
    void addNeighbor(Territory* territory);
    // tells whether the given territory is adjacent to this one
    bool isNeighborTo(const Territory* territory) const;
    // removes all edges of this territory
    void clearNeighbors();
};

/**
 * Continent
 * A connected subgraph of the map.
 */
class Continent {
private:
    std::string* name;
    int bonus;
    std::vector<Territory*>* territories;

public:
    // default constructor
    Continent(const std::string& name, int bonus);
    // copy constructor
    Continent(const Continent& other);
    // destructor
    ~Continent();
    // assignment operator
    Continent& operator=(const Continent& other);
    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Continent& continent);

    // getters/setters
    std::string getName() const;
    int getBonus() const;
    std::vector<Territory*> getTerritories() const;

    // adds a territory to the continent
    void addTerritory(Territory* territory);
    // tells whether the given territory is part of this continent
    bool containsTerritory(const Territory* territory) const;
    // removes all territories of this continent
    void clearTerritories();
};

/**
 * Map
 * A graph of Territories grouped into Continents.
 */
class Map {
private:
    std::vector<Territory*>* territories;
    std::vector<Continent*>* continents;

    // deletes every territory and continent owned by the map
    void clear();
    // deep-copies the territories, edges and continents of another map into this one
    void copyFrom(const Map& other);
    // returns the position of a territory in the territories list, or -1
    int indexOfTerritory(const Territory* territory) const;
    // counts the allowable territories reachable from start
    size_t countReachableTerritories(Territory* start, const std::vector<Territory*>& allowed) const;

public:
    // default constructor
    Map();
    // copy constructor
    Map(const Map& other);
    // destructor
    ~Map();
    // assignment operator
    Map& operator=(const Map& other);
    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const Map& map);

    // getters/setters
    int getTerritoryCount() const;
    int getContinentCount() const;
    Territory* getTerritoryById(int id) const;
    Continent* getContinentAt(int index) const;

    // adds a territory to the map
    void addTerritory(Territory* territory);
    // adds a continent to the map
    void addContinent(Continent* continent);

    // check 1: tells whether every territory can reach every other territory
    bool isMapConnected() const;
    // check 2: tells whether each continent is a connected subgraph
    bool isEachContinentConnected() const;
    // check 3: tells whether each territory belongs to one and only one continent
    bool isEachTerritorySolelyOwned() const;
    // runs the three checks, prints PASS/FAIL for each only if printResults is true
    bool validate(bool printResults) const;
};

/**
 * MapLoader
 * Builds a Map from a Domination .map file.
 */
class MapLoader {
private:
    std::string* lastError;

    // stores the error message, deletes the half-built map and returns nullptr
    Map* reject(Map* map, const std::string& reason);

public:
    // default constructor
    MapLoader();
    // copy constructor
    MapLoader(const MapLoader& other);
    // destructor
    ~MapLoader();
    // assignment operator
    MapLoader& operator=(const MapLoader& other);
    // stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const MapLoader& loader);

    // getters/setters
    std::string getLastError() const;

    // reads the .map file at the given path, returns nullptr if it is rejected
    Map* loadMap(const std::string& path);
};

#endif