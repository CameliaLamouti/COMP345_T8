#include "CommandProcessing.h"
#include <iostream>
#include <string>

// Driver for command processor
int main(int argc, char* argv[]) {
    std::cout << "=== Command Processor Driver ===" << std::endl;

    if (argc < 2) {
        std::cout << "Use: -console   or   -file filename.txt" << std::endl;
        return 1;
    }

    CommandProcessor* proc = nullptr;
    std::string mode = argv[1];

    if (mode == "-console") {
        std::cout << "Mode: console" << std::endl;
        proc = new CommandProcessor();
    } else if (mode == "-file" && argc >= 3) {
        std::cout << "Mode: file (" << argv[2] << ")" << std::endl;
        proc = new FileCommandProcessorAdapter(argv[2]);
    } else {
        std::cout << "Bad arguments" << std::endl;
        return 1;
    }

    std::string states[] = {"start", "maploaded", "mapvalidated", "playersadded", "win"};
    int s = 0;

    std::cout << "\nReading commands (type stop to finish):\n" << std::endl;

    for (int i = 0; i < 6; i++) {
        Command* c = proc->getCommand();
        if (c->getCommand() == "stop" || c->getCommand() == "") break;

        std::string state = states[s % 5];
        bool good = proc->validate(c, state);

        std::cout << *c << std::endl;
        if (good) {
            std::cout << "Accepted" << std::endl;
            s++;
        } else {
            std::cout << "Rejected" << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "\nAll stored commands:" << std::endl;
    std::cout << *proc;

    delete proc;
    std::cout << "Done." << std::endl;
    return 0;
}