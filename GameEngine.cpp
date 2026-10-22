#include "GameEngine.h"
#include <iostream>
#include <string>

// Default constructor: Allocates a new string on the heap and sets the starting state
GameEngine::GameEngine() {
    currentState = new std::string("start");
}

// Copy constructor: Creates a deep copy of the other engine's state
GameEngine::GameEngine(const GameEngine& other) {
    currentState = new std::string(*(other.currentState));
}

// Destructor: Frees the memory allocated for the state string (no memory leaks)
GameEngine::~GameEngine() {
    delete currentState;
    currentState = nullptr;
}

// Assignment operator: Protects against self-assignment and performs a deep copy
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        delete currentState;                              
        currentState = new std::string(*(other.currentState)); 
    }
    return *this;
}

// Stream insertion operator: Prints a readable description of the current state
std::ostream& operator<<(std::ostream& out, const GameEngine& engine) {
    out << "Current state: " << *(engine.currentState);
    return out;
}

// Helper that returns the current state as a normal string
std::string GameEngine::getCurrentState() const {
    return *currentState;
}

// Main method that implements the state transition diagram
void GameEngine::processCommand(const std::string& command) {
    std::string state = *currentState;

    // START
    if (state == "start") {
        if (command == "loadmap") {
            *currentState = "map loaded";
            std::cout << "Transition: start -> map loaded" << std::endl;
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'start'. Expected: loadmap" << std::endl;
        }
    }
    // MAP LOADED
    else if (state == "map loaded") {
        if (command == "loadmap") {
            *currentState = "map loaded";
            std::cout << "Transition: map loaded -> map loaded (reloaded)" << std::endl;
        } else if (command == "validatemap") {
            *currentState = "map validated";
            std::cout << "Transition: map loaded -> map validated" << std::endl;
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'map loaded'. Expected: loadmap or validatemap" << std::endl;
        }
    }
    // MAP VALIDATED 
    else if (state == "map validated") {
        if (command == "addplayer") {
            *currentState = "players added";
            std::cout << "Transition: map validated -> players added" << std::endl;
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'map validated'. Expected: addplayer" << std::endl;
        }
    }
    // PLAYERS ADDED
    else if (state == "players added") {
        if (command == "addplayer") {
            *currentState = "players added";
            std::cout << "Transition: players added -> players added (another player)" << std::endl;
        } else if (command == "assigncountries") {
            *currentState = "assign reinforcement";
            std::cout << "Transition: players added -> assign reinforcement" << std::endl;
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'players added'. Expected: addplayer or assigncountries" << std::endl;
        }
    }
    // ASSIGN REINFORCEMENT 
    else if (state == "assign reinforcement") {
        if (command == "issueorder") {
            *currentState = "issue orders";
            std::cout << "Transition: assign reinforcement -> issue orders" << std::endl;
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'assign reinforcement'. Expected: issueorder" << std::endl;
        }
    }
    // ISSUE ORDERS 
    else if (state == "issue orders") {
        if (command == "issueorder") {
            *currentState = "issue orders";
            std::cout << "Transition: issue orders -> issue orders (another order)" << std::endl;
        } else if (command == "endissueorders") {
            *currentState = "execute orders";
            std::cout << "Transition: issue orders -> execute orders" << std::endl;
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'issue orders'. Expected: issueorder or endissueorders" << std::endl;
        }
    }
    // EXECUTE ORDERS 
    else if (state == "execute orders") {
        if (command == "execorder") {
            *currentState = "execute orders";
            std::cout << "Transition: execute orders -> execute orders (another order)" << std::endl;
        } else if (command == "endexecorders") {
            *currentState = "assign reinforcement";
            std::cout << "Transition: execute orders -> assign reinforcement" << std::endl;
        } else if (command == "win") {
            *currentState = "win";
            std::cout << "Transition: execute orders -> win" << std::endl;
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'execute orders'. Expected: execorder, endexecorders or win" << std::endl;
        }
    }
    // WIN 
    else if (state == "win") {
        if (command == "play") {
            *currentState = "start";
            std::cout << "Transition: win -> start (new game)" << std::endl;
        } else if (command == "end") {
            std::cout << "Game ended. Goodbye!" << std::endl;
            // The driver will exit the loop when it sees this message / state
        } else {
            std::cout << "Error: Invalid command '" << command
                      << "' in state 'win'. Expected: play or end" << std::endl;
        }
    }
    else {
        std::cout << "Error: Unknown state '" << state << "'" << std::endl;
    }
}
