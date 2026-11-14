#ifndef COMMANDPROCESSING_H
#define COMMANDPROCESSING_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

// Holds one command and its effect
class Command {
private:
    std::string* command;
    std::string* effect;

public:
    Command(std::string cmd);
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();
    friend std::ostream& operator<<(std::ostream& out, const Command& c);

    void saveEffect(std::string eff);
    std::string getCommand() const;
    std::string getEffect() const;
};

// Reads lines from a file (used by the adapter)
class FileLineReader {
private:
    std::ifstream* file;

public:
    FileLineReader(std::string filename);
    ~FileLineReader();
    std::string readLineFromFile();
    bool isOpen() const;
};

// Reads commands from console and stores them
class CommandProcessor {
protected:
    std::vector<Command*>* commands;

    virtual std::string readCommand();
    void saveCommand(std::string cmd);

public:
    CommandProcessor();
    CommandProcessor(const CommandProcessor& other);
    CommandProcessor& operator=(const CommandProcessor& other);
    virtual ~CommandProcessor();
    friend std::ostream& operator<<(std::ostream& out, const CommandProcessor& cp);

    Command* getCommand();
    bool validate(Command* cmd, std::string currentState);
};

// Adapter that reads commands from a file instead
class FileCommandProcessorAdapter : public CommandProcessor {
private:
    FileLineReader* flr;

public:
    FileCommandProcessorAdapter(std::string filename);
    ~FileCommandProcessorAdapter();
    std::string readCommand() override;
};

#endif