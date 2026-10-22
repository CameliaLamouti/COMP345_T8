#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <string>
#include <iostream>

/**
 * GameEngine class
 * Controls the flow of the Warzone game using states and transitions.
 */
class GameEngine {
private:
    // Pointer to the current state name
    std::string* currentState;

public:
    // Default constructor
    GameEngine();

    // Copy constructor
    GameEngine(const GameEngine& other);

    // Destructor
    ~GameEngine();

    // Assignment operator
    GameEngine& operator=(const GameEngine& other);

    // Stream insertion operator
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& engine);

    // Processes a command and changes state if the command is valid
    void processCommand(const std::string& command);

    // Returns the current state as a string
    std::string getCurrentState() const;
};

#endif 