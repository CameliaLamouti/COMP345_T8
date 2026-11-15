#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "Cards.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "Map.h"
#include "Player.h"

// Prints the name of a test followed by PASS or FAIL
static void printResult(const std::string& test, bool passed) {
    std::cout << "  " << test << ": " << (passed ? "PASS" : "FAIL") << std::endl;
}

// Runs and prints the startup tests, returns true if all of them passed
static bool verifyGameStart(const GameEngine& engine) {
    Map* map = engine.getMap();
    std::vector<Player*> players = engine.getPlayers();

    bool mapLoaded = map != nullptr;
    bool mapValidated = mapLoaded && map->validate(false);
    bool playerCountValid = players.size() >= GameEngine::MIN_PLAYERS && players.size() <= GameEngine::MAX_PLAYERS;

    // an order of play is set when every player appears exactly once
    std::set<Player*> distinctPlayers(players.begin(), players.end());
    bool orderIsRandom = playerCountValid && distinctPlayers.size() == players.size();

    // fair means every territory is owned and counts differ by at most one
    size_t total = 0;
    size_t smallest = players.empty() ? 0 : players[0]->getTerritories().size();
    size_t largest = smallest;
    bool ownersCorrect = true;
    bool cardsDrawn = !players.empty();
    bool armiesGiven = !players.empty();
    for (Player* player : players) {
        size_t count = player->getTerritories().size();
        total += count;
        smallest = std::min(smallest, count);
        largest = std::max(largest, count);
        for (Territory* territory : player->getTerritories()) {
            if (territory->getOwner() != player) {
                ownersCorrect = false;
            }
        }
        if (player->getHand()->size() != static_cast<size_t>(GameEngine::INITIAL_CARDS)) {
            cardsDrawn = false;
        }
        if (player->getPool() != GameEngine::INITIAL_ARMIES) {
            armiesGiven = false;
        }
    }
    bool fairDistribution = mapLoaded && ownersCorrect
        && total == static_cast<size_t>(map->getTerritoryCount())
        && largest - smallest <= 1;

    bool playPhase = engine.getCurrentState() == GameEngine::ASSIGN_REINFORCEMENT;

    std::cout << std::endl << "Startup tests:" << std::endl;
    printResult("Map is loaded", mapLoaded);
    printResult("Map is validated", mapValidated);
    printResult("Players are between " + std::to_string(GameEngine::MIN_PLAYERS)
                + " and " + std::to_string(GameEngine::MAX_PLAYERS), playerCountValid);
    printResult("Players order is random", orderIsRandom);
    printResult("Territories are fairly distributed", fairDistribution);
    printResult("Each player has drawn " + std::to_string(GameEngine::INITIAL_CARDS) + " cards", cardsDrawn);
    printResult("Each player has " + std::to_string(GameEngine::INITIAL_ARMIES) + " armies", armiesGiven);
    printResult("Game is in the play phase", playPhase);

    return mapLoaded && mapValidated && playerCountValid && orderIsRandom
        && fairDistribution && cardsDrawn && armiesGiven && playPhase;
}

// Runs the startup phase with commands typed in the console, then tests the result
int main() {
    CommandProcessor* processor = new CommandProcessor();
    GameEngine* engine = new GameEngine(processor);

    engine->startupPhase();
    bool passed = verifyGameStart(*engine);

    delete engine;
    delete processor;

    return passed ? 0 : 1;
}