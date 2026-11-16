#include "CommandProcessing.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "GameEngine.h"

// Returns the text without the whitespace at both ends
static std::string trim(const std::string& text) {
    const std::string whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

//Command
Command::Command(std::string cmd) : Subject(), ILoggable() {
    cmd = trim(cmd);
    size_t split = cmd.find_first_of(" \t");

    command = new std::string(cmd.substr(0, split));
    argument = new std::string(split == std::string::npos ? "" : trim(cmd.substr(split + 1)));
    effect = new std::string("");
}

// Copy constructor: the copy is also watched by the same observers (Part 5)
Command::Command(const Command& other) : Subject(other), ILoggable(other) {
    command = new std::string(*(other.command));
    argument = new std::string(*(other.argument));
    effect = new std::string(*(other.effect));
}

Command& Command::operator=(const Command& other) {
    if (this != &other) {
        Subject::operator=(other);
        *command = *(other.command);
        *argument = *(other.argument);
        *effect = *(other.effect);
    }
    return *this;
}

Command::~Command() {
    delete command;
    delete argument;
    delete effect;
}

std::ostream& operator<<(std::ostream& out, const Command& c) {
    out << "Command: " << c.getCommandText() << " | Effect: " << *(c.effect);
    return out;
}

// Saves the effect of the command, then notifies the log observer (Part 5)
void Command::saveEffect(std::string eff) {
    *effect = eff;
    Notify(this);
}

// Part 5: line written in the game log, e.g. "Command's Effect: [loadmap europe.map] Map 'europe.map' loaded."
std::string Command::stringToLog() const {
    return "Command's Effect: [" + getCommandText() + "] " + *effect;
}

std::string Command::getCommand() const {
    return *command;
}

std::string Command::getEffect() const {
    return *effect;
}

std::string Command::getArgument() const {
    return *argument;
}

bool Command::hasArgument() const {
    return !argument->empty();
}

std::string Command::getCommandText() const {
    if (argument->empty()) return *command;
    return *command + " " + *argument;
}

//FileLineReader
FileLineReader::FileLineReader(std::string filename) {
    file = new std::ifstream(filename);
    this->filename = new std::string(filename);
}

FileLineReader::FileLineReader(const FileLineReader& other) {
    file = new std::ifstream(*(other.filename));
    filename = new std::string(*(other.filename));
}

FileLineReader& FileLineReader::operator=(const FileLineReader& other) {
    if (this != &other) {
        delete file;
        *filename = *(other.filename);
        file = new std::ifstream(*filename);
    }
    return *this;
}

FileLineReader::~FileLineReader() {
    if (file->is_open()) file->close();
    delete file;
    delete filename;
}

std::ostream& operator<<(std::ostream& out, const FileLineReader& reader) {
    out << "FileLineReader: " << *(reader.filename) << (reader.isOpen() ? " (open)" : " (not open)");
    return out;
}

std::string FileLineReader::readLineFromFile() {
    std::string line;
    while (std::getline(*file, line)) {
        if (!trim(line).empty()) return line;
    }
    return "";
}

bool FileLineReader::isOpen() const {
    return file->is_open();
}

//CommandProcessor
const std::string CommandProcessor::LOAD_MAP = "loadmap";
const std::string CommandProcessor::VALIDATE_MAP = "validatemap";
const std::string CommandProcessor::ADD_PLAYER = "addplayer";
const std::string CommandProcessor::GAME_START = "gamestart";
const std::string CommandProcessor::REPLAY = "replay";
const std::string CommandProcessor::QUIT = "quit";

CommandProcessor::CommandProcessor() : Subject(), ILoggable() {
    commands = new std::vector<Command*>();
}

// Copy constructor: the copy is also watched by the same observers (Part 5)
CommandProcessor::CommandProcessor(const CommandProcessor& other) : Subject(other), ILoggable(other) {
    commands = new std::vector<Command*>();
    for (Command* c : *(other.commands)) {
        commands->push_back(new Command(*c));
    }
}

CommandProcessor& CommandProcessor::operator=(const CommandProcessor& other) {
    if (this != &other) {
        Subject::operator=(other);
        for (Command* c : *commands) delete c;
        delete commands;

        commands = new std::vector<Command*>();
        for (Command* c : *(other.commands)) {
            commands->push_back(new Command(*c));
        }
    }
    return *this;
}

CommandProcessor::~CommandProcessor() {
    for (Command* c : *commands) delete c;
    delete commands;
}

std::ostream& operator<<(std::ostream& out, const CommandProcessor& cp) {
    out << "Commands stored (" << cp.commands->size() << "):" << std::endl;
    for (Command* c : *(cp.commands)) {
        out << "  " << *c << std::endl;
    }
    return out;
}

std::string CommandProcessor::readCommand() {
    std::string cmd;
    while (true) {
        std::cout << "Enter command: ";
        if (!std::getline(std::cin, cmd)) return "";
        if (!trim(cmd).empty()) return cmd;
    }
}

// Saves the command, then notifies the log observer (Part 5).
// The new command gets the processor's observers so its effect is logged too.
void CommandProcessor::saveCommand(std::string cmd) {
    Command* command = new Command(cmd);
    attachObserversTo(command);
    commands->push_back(command);
    Notify(this);
}

// Part 5: line written in the game log, e.g. "Command: loadmap europe.map"
std::string CommandProcessor::stringToLog() const {
    if (commands->empty()) return "Command: (none)";
    return "Command: " + commands->back()->getCommandText();
}

Command* CommandProcessor::getCommand() {
    std::string cmdStr = readCommand();
    if (cmdStr.empty()) return nullptr;
    saveCommand(cmdStr);
    return commands->back();
}

std::string CommandProcessor::getNextState(const std::string& commandName, const std::string& currentState) const {
    if (commandName == LOAD_MAP && (currentState == GameEngine::START || currentState == GameEngine::MAP_LOADED)) {
        return GameEngine::MAP_LOADED;
    }
    if (commandName == VALIDATE_MAP && currentState == GameEngine::MAP_LOADED) {
        return GameEngine::MAP_VALIDATED;
    }
    if (commandName == ADD_PLAYER && (currentState == GameEngine::MAP_VALIDATED || currentState == GameEngine::PLAYERS_ADDED)) {
        return GameEngine::PLAYERS_ADDED;
    }
    if (commandName == GAME_START && currentState == GameEngine::PLAYERS_ADDED) {
        return GameEngine::ASSIGN_REINFORCEMENT;
    }
    if (commandName == REPLAY && currentState == GameEngine::WIN) {
        return GameEngine::START;
    }
    if (commandName == QUIT && currentState == GameEngine::WIN) {
        return GameEngine::EXIT_PROGRAM;
    }
    return "";
}

bool CommandProcessor::validate(Command* cmd, const std::string& currentState) {
    const std::string name = cmd->getCommand();

    bool known = name == LOAD_MAP || name == VALIDATE_MAP || name == ADD_PLAYER
        || name == GAME_START || name == REPLAY || name == QUIT;
    if (!known) {
        cmd->saveEffect("Error: unknown command '" + name + "'.");
        return false;
    }
    if (getNextState(name, currentState).empty()) {
        cmd->saveEffect("Error: '" + name + "' is not valid in state '" + currentState + "'.");
        return false;
    }
    if ((name == LOAD_MAP || name == ADD_PLAYER) && !cmd->hasArgument()) {
        cmd->saveEffect("Error: '" + name + "' needs an argument.");
        return false;
    }
    return true;
}

//FileCommandProcessorAdapter
FileCommandProcessorAdapter::FileCommandProcessorAdapter(std::string filename) : CommandProcessor() {
    flr = new FileLineReader(filename);
}

FileCommandProcessorAdapter::FileCommandProcessorAdapter(const FileCommandProcessorAdapter& other) : CommandProcessor(other) {
    flr = new FileLineReader(*(other.flr));
}

FileCommandProcessorAdapter& FileCommandProcessorAdapter::operator=(const FileCommandProcessorAdapter& other) {
    if (this != &other) {
        CommandProcessor::operator=(other);
        *flr = *(other.flr);
    }
    return *this;
}

FileCommandProcessorAdapter::~FileCommandProcessorAdapter() {
    delete flr;
}

std::ostream& operator<<(std::ostream& out, const FileCommandProcessorAdapter& adapter) {
    out << *(adapter.flr) << std::endl << static_cast<const CommandProcessor&>(adapter);
    return out;
}

std::string FileCommandProcessorAdapter::readCommand() {
    if (flr->isOpen()) return flr->readLineFromFile();
    return "";
}