#include "CommandProcessing.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

//Command
Command::Command(std::string cmd) {
    command = new std::string(cmd);
    effect = new std::string("");
}

Command::Command(const Command& other) {
    command = new std::string(*(other.command));
    effect = new std::string(*(other.effect));
}

Command& Command::operator=(const Command& other) {
    if (this != &other) {
        delete command;
        delete effect;
        command = new std::string(*(other.command));
        effect = new std::string(*(other.effect));
    }
    return *this;
}

Command::~Command() {
    delete command;
    delete effect;
}

std::ostream& operator<<(std::ostream& out, const Command& c) {
    out << "Command: " << *(c.command) << " | Effect: " << *(c.effect);
    return out;
}

void Command::saveEffect(std::string eff) {
    *effect = eff;
}

std::string Command::getCommand() const {
    return *command;
}

std::string Command::getEffect() const {
    return *effect;
}

//FileLineReader
FileLineReader::FileLineReader(std::string filename) {
    file = new std::ifstream(filename);
}

FileLineReader::~FileLineReader() {
    if (file->is_open()) file->close();
    delete file;
}

std::string FileLineReader::readLineFromFile() {
    std::string line;
    if (std::getline(*file, line)) return line;
    return "";
}

bool FileLineReader::isOpen() const {
    return file->is_open();
}

//CommandProcessor
CommandProcessor::CommandProcessor() {
    commands = new std::vector<Command*>();
}

CommandProcessor::CommandProcessor(const CommandProcessor& other) {
    commands = new std::vector<Command*>();
    for (Command* c : *(other.commands)) {
        commands->push_back(new Command(*c));
    }
}

CommandProcessor& CommandProcessor::operator=(const CommandProcessor& other) {
    if (this != &other) {
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
    std::cout << "Enter command: ";
    std::getline(std::cin, cmd);
    return cmd;
}

void CommandProcessor::saveCommand(std::string cmd) {
    commands->push_back(new Command(cmd));
}

Command* CommandProcessor::getCommand() {
    std::string cmdStr = readCommand();
    saveCommand(cmdStr);
    return commands->back();
}

bool CommandProcessor::validate(Command* cmd, std::string currentState) {
    std::string full = cmd->getCommand();
    std::string first = full.substr(0, full.find(' '));

    bool ok = false;
    if (currentState == "start" && first == "loadmap") ok = true;
    else if (currentState == "maploaded" && (first == "loadmap" || first == "validatemap")) ok = true;
    else if (currentState == "mapvalidated" && first == "addplayer") ok = true;
    else if (currentState == "playersadded" && (first == "addplayer" || first == "gamestart")) ok = true;
    else if (currentState == "win" && (first == "replay" || first == "quit")) ok = true;

    if (ok) {
        cmd->saveEffect("Valid in state " + currentState);
    } else {
        cmd->saveEffect("Invalid command in state " + currentState);
    }
    return ok;
}

//FileCommandProcessorAdapter
FileCommandProcessorAdapter::FileCommandProcessorAdapter(std::string filename) : CommandProcessor() {
    flr = new FileLineReader(filename);
}

FileCommandProcessorAdapter::~FileCommandProcessorAdapter() {
    delete flr;
}

std::string FileCommandProcessorAdapter::readCommand() {
    if (flr->isOpen()) return flr->readLineFromFile();
    return "";
}