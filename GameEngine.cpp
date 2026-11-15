#include "GameEngine.h"

#include <algorithm>
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

    std::cout << "Available maps:" << std::endl;
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

    std::cout << "Order of play:" << std::endl;
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