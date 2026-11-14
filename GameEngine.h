#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <string>
#include <vector>
#include <iostream>

class Map;
class Player;
class Deck;
class Command;
class CommandProcessor;
class FileCommandProcessorAdapter;

// States of the game
enum class GameState {
    START,
    MAP_LOADED,
    MAP_VALIDATED,
    PLAYERS_ADDED,
    ASSIGN_REINFORCEMENT,
    ISSUE_ORDERS,
    EXECUTE_ORDERS,
    WIN
};

// Returns the text of a state
std::string gameStateToString(GameState state);

/**
 * GameEngine class
 * Controls the flow of the Warzone game using states and transitions.
 */
class GameEngine {
private:
    static const std::string MAP_DIRECTORY;   // directory where the map files are stored

    GameState* currentState;
    CommandProcessor* commandProcessor;       // not owned
    Map* map;                                 // owned
    std::vector<Player*>* players;            // owned, in order of play after gamestart
    Deck* deck;                               // owned

    // deletes the map, the players and the deck
    void clearGameData();
    // deep-copies the map, players and deck of another engine
    void copyGameData(const GameEngine& other);
    // prints the file names of the maps in the map directory
    void printAvailableMaps() const;

    // command handlers, each one saves the effect of its command
    void handleLoadMap(Command* command, const std::string& argument);
    void handleValidateMap(Command* command);
    void handleAddPlayer(Command* command, const std::string& argument);
    void handleGameStart(Command* command);

    // steps of the gamestart command
    void distributeTerritories();
    void shufflePlayerOrder();
    void giveInitialArmies();
    void drawInitialCards();

public:
    // Constructor taking a command processor (not owned)
    explicit GameEngine(CommandProcessor* processor);
    
    // Copy constructor
    GameEngine(const GameEngine& other);

    // Destructor
    ~GameEngine();

    // Assignment operator
    GameEngine& operator=(const GameEngine& other);

    // Stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& engine);

    // Returns the current state
    GameState getCurrentState() const;

    // Changes the current state
    void setNewState(GameState newState);

    // Returns the loaded map (nullptr if none)
    Map* getMap() const;

    // Returns the players in order of play
    std::vector<Player*> getPlayers() const;

    // Reads commands until the game is started (loadmap, validatemap, addplayer, gamestart)
    void startupPhase();
};

#endif