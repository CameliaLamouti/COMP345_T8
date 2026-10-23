#include <iostream>
#include <string>
#include <vector>

#include "Map.h"

 // prints every validated loaded map
static void printMaps(const std::vector<Map*>& maps) {
    for (size_t i = 0; i < maps.size(); i++) {
        std::cout << "[Map " << (i + 1) << "]" << std::endl;
        Map* map = maps[i];
        for (int c = 0; c < map->getContinentCount(); c++) {
            Continent* continent = map->getContinentAt(c);
            std::cout << *continent << std::endl;
            for (Territory* territory : continent->getTerritories()) {
                std::cout << "    " << *territory << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

// loads, validates, and prints a list of Maps
int main() {
    std::vector<std::string> filePaths = {
        "maps\\europe.map",
        "maps\\does_not_exist.map",
        "maps\\missing_file.map",
        "maps\\rio_de_janeiro.map",
        "maps\\invalid_configuration.map",
        "maps\\disconnected.map",
        "maps\\WoW.map",
        "maps\\random.txt"
    };

    std::vector<Map*> maps;
    MapLoader loader;

    std::cout << std::endl;
    std::cout << "Loading maps..." << std::endl;
    std::cout << std::endl;
    for (const std::string& path : filePaths) {
        Map* map = loader.loadMap(path);

        if (map == nullptr) {
            std::cout << path << " -> [ERROR: " << loader.getLastError() << "]" << std::endl;
            std::cout << std::endl;
            continue;
        }

        std::cout << path << " -> [SUCCESS]" << std::endl;
        std::cout << "Validating map:" << std::endl;
        if (map->validate(true)) {
            maps.push_back(map);
        } else {
            delete map;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl << "View loaded maps (y/n)?" << std::endl;
    std::string answer;
    std::cin >> answer;

    if (answer == "y" || answer == "Y") {
        std::cout << std::endl;
        printMaps(maps);
    }

    for (Map* map : maps) {
        delete map;
    }

    return 0;
}