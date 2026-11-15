#include "GameEngine.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>

#include "Cards.h"
#include "CommandProcessing.h"
#include "Map.h"
#include "Player.h"

const std::string GameEngine::START = "start";
const std::string GameEngine::MAP_LOADED = "maploaded";
const std::string GameEngine::MAP_VALIDATED = "mapvalidated";
const std::string GameEngine::PLAYERS_ADDED = "playersadded";
const std::string GameEngine::ASSIGN_REINFORCEMENT = "assignreinforcement";
const std::string GameEngine::ISSUE_ORDERS = "issueorders";
const std::string GameEngine::EXECUTE_ORDERS = "executeorders";
const std::string GameEngine::WIN = "win";
const std::string GameEngine::EXIT_PROGRAM = "exitprogram";

const size_t GameEngine::MIN_PLAYERS = 2;
const size_t GameEngine::MAX_PLAYERS = 6;
const int GameEngine::INITIAL_ARMIES = 50;
const int GameEngine::INITIAL_CARDS = 2;
const int GameEngine::MIN_REINFORCEMENT = 3;
const int GameEngine::TERRITORIES_PER_ARMY = 3;
const bool GameEngine::CLEAR_SCREEN = true;

const std::string GameEngine::MAP_DIRECTORY = "maps";

// Returns a random number generator seeded once from the system
static std::mt19937& randomGenerator() {
    static std::mt19937 generator{std::random_device{}()};
    return generator;
}

// Constructor: starts in the start state with no map and no players
GameEngine::GameEngine(CommandProcessor* processor) :
    currentState(new std::string(START)),
    commandProcessor(processor),
    map(nullptr),
    players(new std::vector<Player*>()),
    deck(new Deck()) {}

// Copy constructor: deep copy, the command processor is shared
GameEngine::GameEngine(const GameEngine& other) :
    currentState(new std::string(*other.currentState)),
    commandProcessor(other.commandProcessor),
    map(nullptr),
    players(new std::vector<Player*>()),
    deck(nullptr) {
    copyGameData(other);
}

// Destructor
GameEngine::~GameEngine() {
    clearGameData();
    delete players;
    delete currentState;
}

// Assignment operator: deep copy with a self-assignment check
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        clearGameData();
        *currentState = *other.currentState;
        commandProcessor = other.commandProcessor;
        copyGameData(other);
    }
    return *this;
}

// Stream insertion operator
std::ostream& operator<<(std::ostream& out, const GameEngine& engine) {
    out << "Current state: " << *engine.currentState
        << " (" << engine.players->size() << " players)";
    return out;
}

// Deletes the map, the players and the deck
void GameEngine::clearGameData() {
    for (Player* player : *players) {
        delete player;
    }
    players->clear();
    delete map;
    map = nullptr;
    delete deck;
    deck = nullptr;
}

// Copies the map, players and deck, matching territories by id
void GameEngine::copyGameData(const GameEngine& other) {
    if (other.deck != nullptr) {
        deck = new Deck(*other.deck);
    }

    if (other.map != nullptr) {
        map = new Map(*other.map);
    }

    for (Player* original : *other.players) {
        Player* copy = new Player(original->getName());
        *copy->getHand() = *original->getHand();
        *copy->getOrders() = *original->getOrders();
        copy->setPool(original->getPool());
        if (map != nullptr) {
            for (Territory* territory : original->getTerritories()) {
                Territory* mine = map->getTerritoryById(territory->getId());
                if (mine != nullptr) {
                    copy->addTerritory(mine);
                    mine->setOwner(copy);
                }
            }
        }
        players->push_back(copy);
    }
}

// Clears the console (does nothing when CLEAR_SCREEN is false)
void GameEngine::clearScreen() {
    if (!CLEAR_SCREEN) {
        return;
    }
#ifdef _WIN32
    std::system("cls");
#else
    std::cout << "\033[2J\033[3J\033[H" << std::flush;
#endif
}

// Prints a heading in capital letters between brackets, underlined, after a blank line
void GameEngine::printHeading(const std::string& title) {
    std::string upper = title;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    std::cout << std::endl << "[" << upper << "]" << std::endl
              << std::string(upper.size() + 2, '=') << std::endl;
}

// Shows a message and waits until the Enter key is pressed (or the input ends)
void GameEngine::waitForEnter(const std::string& message) {
    std::cout << std::endl << message << std::flush;
    std::string line;
    std::getline(std::cin, line);
}

// Returns the current state
std::string GameEngine::getCurrentState() const {
    return *currentState;
}

// Changes the current state and prints the transition
void GameEngine::transition(const std::string& newState) {
    std::cout << "Transition: " << *currentState
              << " -> " << newState << std::endl;
    *currentState = newState;
}

// Returns the loaded map (nullptr if none)
Map* GameEngine::getMap() const {
    return map;
}

// Returns the players in order of play
std::vector<Player*> GameEngine::getPlayers() const {
    return *players;
}

// Prints the file names of the maps in the map directory
void GameEngine::printAvailableMaps() const {
    std::vector<std::string> names;
    std::error_code error;
    std::filesystem::directory_iterator it(MAP_DIRECTORY, error);
    if (!error) {
        for (const auto& entry : it) {
            if (entry.is_regular_file()) {
                names.push_back(entry.path().filename().string());
            }
        }
    }
    std::sort(names.begin(), names.end());

    std::cout << std::endl << "Available maps:" << std::endl;
    if (names.empty()) {
        std::cout << "  (none)" << std::endl;
    }
    for (const std::string& name : names) {
        std::cout << "  " << name << std::endl;
    }
}

// Reads commands until the game is started or there are no more commands
void GameEngine::startupPhase() {
    if (commandProcessor == nullptr) {
        std::cout << "Error: no command processor." << std::endl;
        return;
    }

    printAvailableMaps();

    while (*currentState != ASSIGN_REINFORCEMENT) {
        std::cout << std::endl;
        Command* command = commandProcessor->getCommand();
        if (command == nullptr) {
            std::cout << "No more commands." << std::endl;
            return;
        }

        // validate() saves the error message as the effect of an invalid command
        if (!commandProcessor->validate(command, *currentState)) {
            std::cout << command->getEffect() << std::endl;
            continue;
        }

        const std::string name = command->getCommand();
        if (name == CommandProcessor::LOAD_MAP) {
            handleLoadMap(command);
        } else if (name == CommandProcessor::VALIDATE_MAP) {
            handleValidateMap(command);
        } else if (name == CommandProcessor::ADD_PLAYER) {
            handleAddPlayer(command);
        } else if (name == CommandProcessor::GAME_START) {
            handleGameStart(command);
        }
        std::cout << command->getEffect() << std::endl;
    }
}

// Loads a map from the map directory, a failed load keeps the previous map
void GameEngine::handleLoadMap(Command* command) {
    const std::string argument = command->getArgument();

    MapLoader loader;
    Map* loaded = loader.loadMap(MAP_DIRECTORY + "/" + argument);
    if (loaded == nullptr) {
        command->saveEffect("Error: could not load '" + argument + "': " + loader.getLastError());
        return;
    }

    // a new map starts a new game
    clearGameData();
    deck = new Deck();
    map = loaded;
    transition(MAP_LOADED);
    command->saveEffect("Map '" + argument + "' loaded.");
}

// Validates the map, an invalid map stays in the map loaded state
void GameEngine::handleValidateMap(Command* command) {
    if (map == nullptr) {
        command->saveEffect("Error: no map is loaded.");
        return;
    }

    if (!map->validate(true)) {
        command->saveEffect("Error: the map is invalid, load another one.");
        return;
    }
    transition(MAP_VALIDATED);
    command->saveEffect("Map is valid.");
}

// Adds a player, between 2 and 6 players are allowed in a game
void GameEngine::handleAddPlayer(Command* command) {
    const std::string argument = command->getArgument();

    if (players->size() >= MAX_PLAYERS) {
        command->saveEffect("Error: the game already has " + std::to_string(MAX_PLAYERS) + " players.");
        return;
    }
    for (Player* player : *players) {
        if (player->getName() == argument) {
            command->saveEffect("Error: a player named '" + argument + "' already exists.");
            return;
        }
    }

    players->push_back(new Player(argument));
    transition(PLAYERS_ADDED);
    command->saveEffect("Player '" + argument + "' added (" + std::to_string(players->size()) + " in game).");
}

// Sets up the players and switches the game to the play phase
void GameEngine::handleGameStart(Command* command) {
    if (players->size() < MIN_PLAYERS) {
        command->saveEffect("Error: at least " + std::to_string(MIN_PLAYERS) + " players are needed to start the game.");
        return;
    }

    distributeTerritories();
    shufflePlayerOrder();
    giveInitialArmies();
    drawInitialCards();
    transition(ASSIGN_REINFORCEMENT);
    command->saveEffect("Game started.");

    std::cout << std::endl << "Order of play:" << std::endl;
    for (size_t i = 0; i < players->size(); i++) {
        Player* player = (*players)[i];
        std::cout << "  " << (i + 1) << ". " << player->getName()
                  << " - territories: " << player->getTerritories().size()
                  << ", pool: " << player->getPool()
                  << ", hand: " << *player->getHand() << std::endl;
    }
}

// Deals the shuffled territories to the players one by one
void GameEngine::distributeTerritories() {
    std::vector<Territory*> toDeal = map->getTerritories();
    std::shuffle(toDeal.begin(), toDeal.end(), randomGenerator());

    for (size_t i = 0; i < toDeal.size(); i++) {
        Player* player = (*players)[i % players->size()];
        toDeal[i]->setOwner(player);
        player->addTerritory(toDeal[i]);
    }
}

// Randomly decides the order of play
void GameEngine::shufflePlayerOrder() {
    std::shuffle(players->begin(), players->end(), randomGenerator());
}

// Puts the initial armies in every player's pool
void GameEngine::giveInitialArmies() {
    for (Player* player : *players) {
        player->setPool(INITIAL_ARMIES);
    }
}

// Every player draws their initial cards from the deck
void GameEngine::drawInitialCards() {
    for (Player* player : *players) {
        for (int i = 0; i < INITIAL_CARDS; i++) {
            deck->draw(player->getHand());
        }
    }
}

// Plays rounds until one player owns every territory, then announces the winner.
// The console is cleared at the start of every phase and every player's turn.
void GameEngine::mainGameLoop() {
    if (map == nullptr || players->empty()) {
        std::cout << std::endl << "Error: the game cannot be played without a map and players." << std::endl;
        return;
    }

    refreshPlayerTerritories();
    removeEliminatedPlayers();

    int rounds = 0;
    while (true) {
        Player* winner = getWinner();
        if (winner != nullptr) {
            if (rounds > 0) {
                clearScreen();
            }
            printHeading("Game over");
            std::cout << std::endl << winner->getName()
                      << " owns every territory and wins the game!" << std::endl;
            transition(WIN);
            return;
        }

        if (std::cin.eof()) {
            std::cout << std::endl << "The input ended, the game is stopped." << std::endl;
            return;
        }

        rounds++;
        clearScreen();
        reinforcementPhase();
        waitForEnter("Press Enter to go to the issue orders phase...");

        issueOrdersPhase();

        executeOrdersPhase();
        removeEliminatedPlayers();
        waitForEnter("Press Enter to start the next round...");
    }
}

// Gives every player their armies, in the order of play
void GameEngine::reinforcementPhase() {
    printHeading("Reinforcement phase");
    if (*currentState != ASSIGN_REINFORCEMENT) {
        transition(ASSIGN_REINFORCEMENT);
    }

    std::cout << std::endl;
    for (Player* player : *players) {
        int armies = calculateReinforcement(player);
        player->setPool(player->getPool() + armies);
        std::cout << player->getName() << " receives " << armies
                  << " armies (territories: " << player->getTerritories().size()
                  << "), pool: " << player->getPool() << std::endl;
    }
}

// Asks every player for one order at a time until all of them are done.
// Every player's turn is shown on a cleared console.
void GameEngine::issueOrdersPhase() {
    clearScreen();
    printHeading("Issue orders phase");
    transition(ISSUE_ORDERS);

    for (Player* player : *players) {
        player->resetPlannedArmies();
    }

    // a player that has no more orders to issue is not asked again this turn
    std::vector<bool> done(players->size(), false);
    size_t remaining = players->size();
    waitForEnter("Press Enter to start the first turn...");
    while (remaining > 0 && !std::cin.eof()) {
        for (size_t i = 0; i < players->size(); i++) {
            if (done[i]) {
                continue;
            }

            Player* player = (*players)[i];
            clearScreen();
            printHeading(player->getName() + "'s turn");
            if (!player->issueOrder(*players, deck)) {
                done[i] = true;
                remaining--;
                std::cout << std::endl << player->getName() << " has no more orders this turn." << std::endl;
            }
            waitForEnter();
        }
    }
}

// Executes all the deploy orders, then every other order, in round-robin fashion
void GameEngine::executeOrdersPhase() {
    clearScreen();
    printHeading("Execute orders phase");
    transition(EXECUTE_ORDERS);
    std::cout << std::endl;

    std::vector<Player*> conquerors;

    // every deploy order is executed before any other kind of order
    bool executed = true;
    while (executed) {
        executed = false;
        for (Player* player : *players) {
            OrdersList* orders = player->getOrders();
            for (int i = 0; i < orders->size(); i++) {
                if (dynamic_cast<Deploy*>(orders->get(i)) != nullptr) {
                    executeOrderAt(player, i, conquerors);
                    executed = true;
                    break;
                }
            }
        }
    }

    // then the top order of each player's list, one player after the other
    executed = true;
    while (executed) {
        executed = false;
        for (Player* player : *players) {
            if (player->getOrders()->size() > 0) {
                executeOrderAt(player, 0, conquerors);
                executed = true;
            }
        }
    }

    rewardConquerors(conquerors);
}

// Executes one order, prints its effect and removes it from the player's list
void GameEngine::executeOrderAt(Player* player, int index, std::vector<Player*>& conquerors) {
    OrdersList* orders = player->getOrders();
    Order* order = orders->get(index);

    // the owners before the order tell whether it conquered a territory
    std::vector<Territory*> territories = map->getTerritories();
    std::vector<Player*> ownersBefore;
    for (Territory* territory : territories) {
        ownersBefore.push_back(territory->getOwner());
    }

    std::cout << player->getName() << " executes: " << order->getDescription() << std::endl;
    order->execute();
    if (!order->getEffect().empty()) {
        std::cout << "  " << order->getEffect() << std::endl;
    }

    refreshPlayerTerritories();

    for (size_t i = 0; i < territories.size(); i++) {
        if (territories[i]->getOwner() == player && ownersBefore[i] != player) {
            if (std::find(conquerors.begin(), conquerors.end(), player) == conquerors.end()) {
                conquerors.push_back(player);
            }
            break;
        }
    }

    orders->remove(index);
}

// A player who conquered at least one territory draws exactly one card
void GameEngine::rewardConquerors(const std::vector<Player*>& conquerors) {
    if (!conquerors.empty()) {
        std::cout << std::endl;
    }
    for (Player* player : conquerors) {
        Card* card = deck->draw(player->getHand());
        if (card != nullptr) {
            std::cout << player->getName() << " conquered a territory and draws a card: " << *card << std::endl;
        } else {
            std::cout << player->getName() << " conquered a territory but the deck is empty." << std::endl;
        }
    }
}

// Territories / 3 (rounded down) plus the bonus of each continent entirely owned, never less than 3
int GameEngine::calculateReinforcement(const Player* player) const {
    int armies = static_cast<int>(player->getTerritories().size()) / TERRITORIES_PER_ARMY;

    if (map != nullptr) {
        for (int i = 0; i < map->getContinentCount(); i++) {
            Continent* continent = map->getContinentAt(i);
            std::vector<Territory*> members = continent->getTerritories();
            if (members.empty()) {
                continue;
            }
            bool ownsAll = true;
            for (Territory* territory : members) {
                if (territory->getOwner() != player) {
                    ownsAll = false;
                    break;
                }
            }
            if (ownsAll) {
                armies += continent->getBonus();
            }
        }
    }
    return std::max(armies, MIN_REINFORCEMENT);
}

// The map knows the owner of each territory: the players' lists are rebuilt from it
void GameEngine::refreshPlayerTerritories() {
    if (map == nullptr) {
        return;
    }
    for (Player* player : *players) {
        player->clearTerritories();
    }
    for (Territory* territory : map->getTerritories()) {
        Player* owner = territory->getOwner();
        // the owner can also be the Neutral player, who is not part of the game's players
        if (std::find(players->begin(), players->end(), owner) != players->end()) {
            owner->addTerritory(territory);
        }
    }
}

// Deletes the players without territory, the others keep their order of play
void GameEngine::removeEliminatedPlayers() {
    refreshPlayerTerritories();
    for (size_t i = 0; i < players->size();) {
        Player* player = (*players)[i];
        if (player->getTerritories().empty()) {
            std::cout << std::endl << player->getName() << " controls no territory and is removed from the game." << std::endl;
            delete player;
            players->erase(players->begin() + i);
        } else {
            i++;
        }
    }
}

// The winner owns every territory of the map
Player* GameEngine::getWinner() const {
    if (map == nullptr || map->getTerritoryCount() == 0) {
        return nullptr;
    }
    for (Player* player : *players) {
        if (static_cast<int>(player->getTerritories().size()) == map->getTerritoryCount()) {
            return player;
        }
    }
    return nullptr;
}