#include <algorithm>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
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

    GameEngine::printHeading("Startup tests");
    std::cout << std::endl;
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

// Runs an action of the game engine without showing what it prints,
// so the tests only show their own results
static void runSilently(const std::function<void()>& action) {
    std::ostringstream buffer;
    std::streambuf* original = std::cout.rdbuf(buffer.rdbuf());
    action();
    std::cout.rdbuf(original);
}

// Gives each territory of the map to the player at the same position in owners
static void stageOwnership(GameEngine& engine, const std::vector<Player*>& owners) {
    std::vector<Territory*> territories = engine.getMap()->getTerritories();
    for (size_t i = 0; i < territories.size(); i++) {
        territories[i]->setOwner(owners[i]);
    }
    engine.refreshPlayerTerritories();
}

// Computes the armies a player should receive straight from the owners stored in the map
static int expectedReinforcement(const Map* map, const Player* player) {
    int owned = 0;
    for (Territory* territory : map->getTerritories()) {
        if (territory->getOwner() == player) {
            owned++;
        }
    }

    int armies = owned / 3;
    for (int i = 0; i < map->getContinentCount(); i++) {
        Continent* continent = map->getContinentAt(i);
        bool ownsAll = !continent->getTerritories().empty();
        for (Territory* territory : continent->getTerritories()) {
            if (territory->getOwner() != player) {
                ownsAll = false;
            }
        }
        if (ownsAll) {
            armies += continent->getBonus();
        }
    }
    return std::max(armies, 3);
}

// Demonstrates that players receive the right number of armies in different situations
// (a copy of the game is used so the real game is not changed)
static bool testReinforcement(const GameEngine& engine) {
    GameEngine game(engine);
    Map* map = game.getMap();
    std::vector<Player*> players = game.getPlayers();
    std::vector<Territory*> territories = map->getTerritories();

    std::vector<std::pair<std::string, std::vector<Player*>>> scenarios;

    std::vector<Player*> alternate;
    for (size_t i = 0; i < territories.size(); i++) {
        alternate.push_back(players[i % 2]);
    }
    scenarios.push_back({"territories split between the first two players", alternate});

    std::vector<Player*> almostAll(territories.size(), players[0]);
    almostAll[0] = players[1];
    scenarios.push_back({"one player owns a single territory (minimum of 3 armies)", almostAll});

    if (map->getContinentCount() > 0) {
        std::vector<Player*> firstContinent;
        for (Territory* territory : territories) {
            firstContinent.push_back(map->getContinentAt(0)->containsTerritory(territory) ? players[0] : players[1]);
        }
        scenarios.push_back({"the first player owns the whole first continent (continent bonus)", firstContinent});
    }

    GameEngine::printHeading("Reinforcement tests");
    bool allPassed = true;
    for (const auto& scenario : scenarios) {
        std::cout << std::endl << "Case: " << scenario.first << std::endl;
        stageOwnership(game, scenario.second);

        std::vector<int> poolsBefore;
        std::vector<int> expected;
        for (Player* player : players) {
            poolsBefore.push_back(player->getPool());
            expected.push_back(expectedReinforcement(map, player));
        }

        runSilently([&game]() { game.reinforcementPhase(); });

        for (size_t i = 0; i < players.size(); i++) {
            int received = players[i]->getPool() - poolsBefore[i];
            bool passed = received == expected[i];
            allPassed = allPassed && passed;
            printResult(players[i]->getName() + " owns " + std::to_string(players[i]->getTerritories().size())
                        + " territories, expects " + std::to_string(expected[i])
                        + " armies, received " + std::to_string(received), passed);
        }
    }
    return allPassed;
}

// Demonstrates that a player without territory is removed and that the game ends when
// one player owns every territory (a copy of the game is used so the real game is not changed)
static bool testEliminationAndWin(const GameEngine& engine) {
    GameEngine game(engine);
    std::vector<Player*> players = game.getPlayers();
    Player* winner = players[0];
    const std::string winnerName = winner->getName();
    const size_t territoryCount = game.getMap()->getTerritoryCount();

    GameEngine::printHeading("Elimination/win tests");
    stageOwnership(game, std::vector<Player*>(territoryCount, winner));

    // what each player owns before the players without territory are removed
    std::vector<std::string> names;
    std::vector<size_t> counts;
    for (Player* player : players) {
        names.push_back(player->getName());
        counts.push_back(player->getTerritories().size());
    }

    runSilently([&game]() { game.removeEliminatedPlayers(); });
    std::vector<std::string> remainingNames;
    for (Player* player : game.getPlayers()) {
        remainingNames.push_back(player->getName());
    }

    bool allPassed = true;
    std::cout << std::endl << "Case: players without any territory are removed from the game" << std::endl;
    for (size_t i = 0; i < names.size(); i++) {
        bool stillPlaying = std::find(remainingNames.begin(), remainingNames.end(), names[i]) != remainingNames.end();
        bool shouldStay = counts[i] > 0;
        bool passed = stillPlaying == shouldStay;
        allPassed = allPassed && passed;
        printResult(names[i] + " owns " + std::to_string(counts[i]) + " territories, expects to "
                    + (shouldStay ? "stay in the game" : "be removed from the game"), passed);
    }

    bool winnerFound = game.getWinner() != nullptr && game.getWinner()->getName() == winnerName;
    runSilently([&game]() { game.mainGameLoop(); });
    bool ended = game.getCurrentState() == GameEngine::WIN;
    bool winPassed = winnerFound && ended;
    allPassed = allPassed && winPassed;

    std::cout << std::endl << "Case: the game ends when one player owns every territory" << std::endl;
    printResult(winnerName + " owns " + std::to_string(territoryCount) + " territories, expects to win the game",
                winPassed);

    return allPassed;
}

// Runs the startup phase with commands typed in the console, tests the startup and
// the reinforcement, elimination and win rules, then plays the main game loop by console
int main() {
    CommandProcessor* processor = new CommandProcessor();
    GameEngine* engine = new GameEngine(processor);

    GameEngine::printHeading("Startup phase");
    engine->startupPhase();
    bool passed = verifyGameStart(*engine);

    if (passed) {
        passed = testReinforcement(*engine) && passed;
        passed = testEliminationAndWin(*engine) && passed;

        GameEngine::waitForEnter("Press Enter to continue to the main game loop...");
        GameEngine::clearScreen();
        GameEngine::printHeading("Main game loop");
        std::cout << std::endl
                  << "Type your orders in the console:" << std::endl
                  << "  - choose the territories to defend and to attack at the start of your turn" << std::endl
                  << "  - while your pool has armies, only deploy orders are accepted" << std::endl
                  << "  - then advance armies to defend or attack, or play a card" << std::endl
                  << "  - choose 0 to end your turn" << std::endl
                  << "The console is cleared between turns. The game ends when one player owns every territory." << std::endl;
        GameEngine::waitForEnter("Press Enter to start the main game loop...");
        engine->mainGameLoop();
    }

    delete engine;
    delete processor;

    return passed ? 0 : 1;
}