// LoggingObserverDriver.cpp
// COMP 345 - Assignment 2 - Part 5 driver: game log observer (commands and orders).
//
// Demonstrates that:
//  (1) Command, CommandProcessor, Order, OrdersList and GameEngine are all
//      subclasses of Subject and ILoggable;
//  (2) CommandProcessor::saveCommand(), Command::saveEffect(), OrdersList::addOrder(),
//      Order::execute() and GameEngine::transition() use Notify() to write log entries;
//  (3) gamelog.txt is written when commands are entered on the console (or read from a file);
//  (4) adding an order to a player's list writes the order in the log;
//  (5) executing an order writes its effect in the log;
//  (6) every GameEngine state change writes the new state in the log.
//
// How to use:
// 1) Compile: g++ -std=c++17 LoggingObserverDriver.cpp LoggingObserver.cpp CommandProcessing.cpp GameEngine.cpp Map.cpp Player.cpp Cards.cpp Orders.cpp -o logging_driver
// 2) Run:     ./logging_driver -console                 (type the startup commands)
//             ./logging_driver -file part5commands.txt  (read the startup commands from a file)
//    The "maps" folder must be in the folder the program is run from.

#include "Cards.h"
#include "CommandProcessing.h"
#include "GameEngine.h"
#include "LoggingObserver.h"
#include "Map.h"
#include "Orders.h"
#include "Player.h"

#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

/**
 * Prints the lines of the log file after the first linesAlreadyShown lines,
 * and returns the total number of lines in the file.
 */
static int printNewLogLines(const std::string& fileName, int linesAlreadyShown) {
    std::ifstream file(fileName);
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (lineNumber > linesAlreadyShown) {
            std::cout << "    | " << line << "\n";
        }
    }
    if (lineNumber == linesAlreadyShown) {
        std::cout << "    | (no new entry)\n";
    }
    return lineNumber;
}

/**
 * Prints whether a class is a subclass of both Subject and ILoggable.
 */
template <typename T>
static void printSubclassCheck(const std::string& className) {
    std::cout << "  " << className << " is a Subject: "
              << (std::is_base_of<Subject, T>::value ? "yes" : "no")
              << ", is an ILoggable: "
              << (std::is_base_of<ILoggable, T>::value ? "yes" : "no") << "\n";
}

/**
 * Returns a territory owned by someone else than the player that is adjacent
 * to one of the player's territories, or nullptr if there is none.
 */
static Territory* findAdjacentEnemyTerritory(Player* player) {
    for (Territory* own : player->getTerritories()) {
        for (Territory* neighbor : own->getNeighbors()) {
            if (neighbor->getOwner() != nullptr && neighbor->getOwner() != player) {
                return neighbor;
            }
        }
    }
    return nullptr;
}

/**
 * Gives a territory to a player (owner and player's list).
 */
static void giveTerritory(Territory* territory, Player* player) {
    territory->setOwner(player);
    player->addTerritory(territory);
}

/**
 * Builds a small test map (owned by the caller) with two players, used only
 * when the startup phase did not start a game. The players' orders lists are
 * attached to the log observer here because no GameEngine created them.
 */
static void buildFallbackGame(std::vector<Territory*>& territories, std::vector<Player*>& players,
                              Observer* logger) {
    Player* alice = new Player("Alice");
    Player* bob = new Player("Bob");
    Territory* alaska = new Territory("Alaska", 1);
    Territory* alberta = new Territory("Alberta", 2);
    Territory* ontario = new Territory("Ontario", 3);

    alaska->addNeighbor(alberta);
    alberta->addNeighbor(alaska);
    alberta->addNeighbor(ontario);
    ontario->addNeighbor(alberta);

    giveTerritory(alaska, alice);
    giveTerritory(alberta, alice);
    giveTerritory(ontario, bob);
    ontario->setArmies(8);

    alice->getOrders()->Attach(logger);
    bob->getOrders()->Attach(logger);

    territories = {alaska, alberta, ontario};
    players = {alice, bob};
}

/**
 * Runs the Part 5 demonstration with the given command processor, which this
 * function deletes at the end (before the log observer it is attached to).
 */
void testLoggingObserver(CommandProcessor* processor) {
    const std::string logFile = LogObserver::DEFAULT_FILE_NAME;
    int linesShown = 0;

    GameEngine::printHeading("Part 5: game log observer");

    // The single log observer of the game: starts a new, empty gamelog.txt.
    LogObserver* logger = new LogObserver();
    std::cout << *logger << "\n\n";

    // ---------------------------------------------------------------- (1)
    std::cout << "--- (1) Classes that can be logged ---\n";
    printSubclassCheck<Command>("Command");
    printSubclassCheck<CommandProcessor>("CommandProcessor");
    printSubclassCheck<FileCommandProcessorAdapter>("FileCommandProcessorAdapter");
    printSubclassCheck<Order>("Order");
    printSubclassCheck<Deploy>("Deploy (an Order)");
    printSubclassCheck<OrdersList>("OrdersList");
    printSubclassCheck<GameEngine>("GameEngine");
    std::cout << "\n";

    // ---------------------------------------------------------------- (2) (3) (6)
    GameEngine* engine = new GameEngine(processor);
    processor->Attach(logger);
    engine->Attach(logger);
    std::cout << "Command processor: attached to " << processor->getObserverCount() << " observer(s)\n";
    std::cout << "Game engine:       attached to " << engine->getObserverCount() << " observer(s)\n\n";

    std::cout << "--- (2)(3)(6) Startup phase: commands, their effects and state changes ---\n";
    std::cout << "Example: loadmap europe.map, validatemap, addplayer Alice, addplayer Bob, gamestart\n";
    engine->startupPhase();
    std::cout << "\nNew entries in " << logFile << ":\n";
    std::cout << "  'Command:'               written by CommandProcessor::saveCommand() -> Notify()\n";
    std::cout << "  'Command's Effect:'      written by Command::saveEffect()           -> Notify()\n";
    std::cout << "  'Game Engine new state:' written by GameEngine::transition()        -> Notify()\n";
    linesShown = printNewLogLines(logFile, linesShown);
    std::cout << "\n";

    // Players for the orders part: the engine's players, or a small test game.
    std::vector<Player*> players = engine->getPlayers();
    std::vector<Territory*> fallbackTerritories;
    std::vector<Player*> fallbackPlayers;
    if (engine->getCurrentState() != GameEngine::ASSIGN_REINFORCEMENT || players.size() < 2) {
        std::cout << "The game was not started, so the orders are shown on a small test map.\n\n";
        buildFallbackGame(fallbackTerritories, fallbackPlayers, logger);
        players = fallbackPlayers;
    }
    Player* player = players[0];
    Player* enemy = players[1];
    OrdersList* orders = player->getOrders();

    // ---------------------------------------------------------------- (4)
    std::cout << "--- (4) Orders added to " << player->getName() << "'s orders list ---\n";
    std::cout << player->getName() << "'s orders list: attached to " << orders->getObserverCount()
              << (fallbackPlayers.empty() ? " observer(s), given by the game engine when the player was added\n"
                                          : " observer(s)\n");
    engine->transition(GameEngine::ISSUE_ORDERS);

    Territory* own = player->getTerritories().front();
    Territory* otherOwn = player->getTerritories().back();
    Territory* neighbor = own->getNeighbors().empty() ? otherOwn : own->getNeighbors().front();
    Territory* enemyTarget = findAdjacentEnemyTerritory(player);

    // An order made by playing a card also goes through addOrder().
    Deck* deck = new Deck();
    if (player->getHand()->isEmpty()) {
        deck->draw(player->getHand());
    }
    player->getHand()->getCard(0)->play(player, deck);

    // One order of every kind, plus an invalid one, added with OrdersList::addOrder().
    orders->addOrder(new Deploy(player, own, 5));
    orders->addOrder(new Advance(player, own, neighbor, 3));
    orders->addOrder(new Bomb(player, enemyTarget));
    orders->addOrder(new Airlift(player, own, otherOwn, 1));
    orders->addOrder(new Blockade(player, otherOwn));
    orders->addOrder(new Negotiate(player, enemy));
    orders->addOrder(new Deploy(player, own, 0));   // invalid: 0 armies

    std::cout << "\nNew entries in " << logFile << " ('Order Issued:' written by OrdersList::addOrder() -> Notify()):\n";
    linesShown = printNewLogLines(logFile, linesShown);
    std::cout << "\n";

    // ---------------------------------------------------------------- (5)
    std::cout << "--- (5) Executing every order in " << player->getName() << "'s orders list ---\n";
    engine->transition(GameEngine::EXECUTE_ORDERS);
    for (int i = 0; i < orders->size(); ++i) {
        orders->get(i)->execute();
    }
    std::cout << *orders << "\n";
    std::cout << "\nNew entries in " << logFile << " ('Order Executed:' written by Order::execute() -> Notify()):\n";
    linesShown = printNewLogLines(logFile, linesShown);
    std::cout << "\n";

    // ---------------------------------------------------------------- whole log
    std::cout << "--- Full content of " << logFile << " ---\n";
    printNewLogLines(logFile, 0);

    // Clean up: the subjects first, then the observer they point to.
    delete deck;
    delete engine;      // also deletes the map and the players it created
    delete processor;   // also deletes the commands it saved
    for (Player* p : fallbackPlayers) delete p;
    for (Territory* t : fallbackTerritories) delete t;
    delete logger;
}

/**
 * Chooses where the commands come from (-console or -file <filename>) and
 * runs the demonstration.
 */
int main(int argc, char* argv[]) {
    CommandProcessor* processor = nullptr;
    std::string mode = argc >= 2 ? argv[1] : "-console";

    if (mode == "-console") {
        std::cout << "Mode: console\n";
        processor = new CommandProcessor();
    } else if (mode == "-file" && argc >= 3) {
        std::ifstream check(argv[2]);
        if (!check.is_open()) {
            std::cout << "Could not open " << argv[2] << "\n";
            return 1;
        }
        std::cout << "Mode: file (" << argv[2] << ")\n";
        processor = new FileCommandProcessorAdapter(argv[2]);
    } else {
        std::cout << "Use: " << argv[0] << " -console   or   " << argv[0] << " -file <filename>\n";
        return 1;
    }

    testLoggingObserver(processor);   // deletes the processor
    return 0;
}
