#include "GameEngine.h"
#include <iostream>
#include <string>

/**
 * GameEngineDriver
 * Demonstrates that the GameEngine correctly handles all valid
 * state transitions shown in the assignment diagram and rejects
 * invalid commands.
 *
 * How to use:
 *   - Compile: g++ GameEngine.cpp GameEngineDriver.cpp -o GameEngineDriver
 *   - Run:     ./GameEngineDriver
 *   - Type the commands from the diagram (loadmap, validatemap, etc.)
 *   - Type "end" while in the "win" state to quit.
 */
int main() {
    // Create the engine on the heap (pointer style consistent with the assignment)
    GameEngine* engine = new GameEngine();

    std::cout << "=== Warzone Game Engine Driver ===" << std::endl;
    std::cout << "Type commands to move between states." << std::endl;
    std::cout << "Valid path example:" << std::endl;
    std::cout << "  loadmap -> validatemap -> addplayer -> assigncountries" << std::endl;
    std::cout << "  -> issueorder -> endissueorders -> execorder -> win -> play (or end)" << std::endl;
    std::cout << "Type 'end' when you are in the win state to quit." << std::endl;
    std::cout << "===================================" << std::endl << std::endl;

    std::string command;

    // Keep reading commands until the user ends the game
    while (true) {
        std::cout << *engine << std::endl;
        std::cout << "Enter command: ";
        std::cin >> command;

        // Special case: quit only allowed from the win state
        if (command == "end" && engine->getCurrentState() == "win") {
            engine->processCommand(command);
            break;
        }

        engine->processCommand(command);
        std::cout << std::endl;
    }

    // Clean up memory
    delete engine;
    engine = nullptr;

    std::cout << "Driver finished. No memory leaks." << std::endl;
    return 0;
}
