#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <string>
#include <vector>
#include <iostream>

#include "LoggingObserver.h"

class Map;
class Player;
class Deck;
class Command;
class CommandProcessor;
class FileCommandProcessorAdapter;

/**
 * GameEngine class
 * Controls the flow of the Warzone game using states and transitions.
 * Part 5: the GameEngine is a Subject and an ILoggable, transition() notifies
 * the log observer with the new state.
 */
class GameEngine : public Subject, public ILoggable {
public:
    // States of the game
    static const std::string START;
    static const std::string MAP_LOADED;
    static const std::string MAP_VALIDATED;
    static const std::string PLAYERS_ADDED;
    static const std::string ASSIGN_REINFORCEMENT;
    static const std::string ISSUE_ORDERS;
    static const std::string EXECUTE_ORDERS;
    static const std::string WIN;
    static const std::string EXIT_PROGRAM;

    static const size_t MIN_PLAYERS;
    static const size_t MAX_PLAYERS;
    static const int INITIAL_ARMIES;
    static const int INITIAL_CARDS;
    static const int MIN_REINFORCEMENT;       // minimum armies a player receives each turn
    static const int TERRITORIES_PER_ARMY;    // territories needed to earn one reinforcement army
    static const bool CLEAR_SCREEN;           // tells whether clearScreen() really clears the console

private:
    static const std::string MAP_DIRECTORY;   // directory where the map files are stored

    std::string* currentState;
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
    void handleLoadMap(Command* command);
    void handleValidateMap(Command* command);
    void handleAddPlayer(Command* command);
    void handleGameStart(Command* command);

    // steps of the gamestart command
    void distributeTerritories();
    void shufflePlayerOrder();
    void giveInitialArmies();
    void drawInitialCards();

    // executes the order at a position of the player's list, then removes it from the list
    // (a player who conquered a territory is added to conquerors)
    void executeOrderAt(Player* player, int index, std::vector<Player*>& conquerors);
    // gives a card to every player who conquered at least one territory this turn
    void rewardConquerors(const std::vector<Player*>& conquerors);

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
    std::string getCurrentState() const;

    // Changes the current state and notifies the log observer (Part 5)
    void transition(const std::string& newState);

    // Part 5: line written in the game log when the state changes
    std::string stringToLog() const override;

    // Returns the loaded map (nullptr if none)
    Map* getMap() const;

    // Returns the players in order of play
    std::vector<Player*> getPlayers() const;

    // Console helpers used to keep the output readable
    static void clearScreen();                                                  // Clears the console
    static void printHeading(const std::string& title);                         // Prints a section heading such as [MAIN GAME LOOP]
    static void waitForEnter(const std::string& message = "Press Enter to continue..."); // Shows a message and waits for the Enter key

    // Reads commands until the game is started (loadmap, validatemap, addplayer, gamestart)
    void startupPhase();

    // Plays rounds of reinforcement, issuing orders and orders execution until one player owns every territory
    void mainGameLoop();

    // Gives every player the armies they earn this turn
    void reinforcementPhase();

    // Asks every player for orders in round-robin fashion until nobody has any more to issue
    void issueOrdersPhase();

    // Executes the orders of every player (deploy orders first) in round-robin fashion
    void executeOrdersPhase();

    // Returns the armies a player earns this turn: territories / 3 plus the bonus of every continent owned, at least 3
    int calculateReinforcement(const Player* player) const;

    // Rebuilds the territory list of every player from the owners stored in the map
    void refreshPlayerTerritories();

    // Removes (and deletes) the players that do not control any territory
    void removeEliminatedPlayers();

    // Returns the player who owns every territory of the map, nullptr if there is none
    Player* getWinner() const;
};

#endif