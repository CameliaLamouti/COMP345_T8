#include "CommandProcessing.h"
#include "GameEngine.h"
#include <iostream>
#include <string>

static const std::string CONSOLE_FLAG = "-console";
static const std::string FILE_FLAG = "-file";
static const std::string STOP_WORD = "stop";

// Driver for command processor
int main(int argc, char* argv[]) {
    std::cout << "=== Command Processor Driver ===" << std::endl;

    if (argc < 2) {
        std::cout << "Use: " << CONSOLE_FLAG << "   or   " << FILE_FLAG << " filename.txt" << std::endl;
        return 1;
    }

    CommandProcessor* proc = nullptr;
    std::string mode = argv[1];

    if (mode == CONSOLE_FLAG) {
        std::cout << "Mode: console" << std::endl;
        proc = new CommandProcessor();
    } else if (mode == FILE_FLAG && argc >= 3) {
        std::cout << "Mode: file (" << argv[2] << ")" << std::endl;
        proc = new FileCommandProcessorAdapter(argv[2]);
    } else {
        std::cout << "Bad arguments" << std::endl;
        return 1;
    }

    std::string state = GameEngine::START;

    std::cout << "\nReading commands (type " << STOP_WORD << " to finish):\n" << std::endl;

    while (true) {
        Command* c = proc->getCommand();
        if (c == nullptr || c->getCommand() == STOP_WORD) break;

        if (proc->validate(c, state)) {
            std::string next = proc->getNextState(c->getCommand(), state);
            c->saveEffect("Accepted: " + state + " -> " + next);
            state = next;
        }

        std::cout << *c << std::endl;
        std::cout << std::endl;

        if (state == GameEngine::EXIT_PROGRAM) break;
    }

    std::cout << "\nAll stored commands:" << std::endl;
    std::cout << *proc;

    delete proc;
    std::cout << "Done." << std::endl;
    return 0;
}