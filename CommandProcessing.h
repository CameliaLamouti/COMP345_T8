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
    std::string* argument;
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

    // Returns the argument of the command, empty if there is none
    std::string getArgument() const;
    // Returns true if the command has an argument
    bool hasArgument() const;
    // Returns the command and its argument as typed
    std::string getCommandText() const;
};

// Reads lines from a file (used by the adapter)
class FileLineReader {
private:
    std::ifstream* file;
    std::string* filename;

public:
    FileLineReader(std::string filename);
    FileLineReader(const FileLineReader& other);
    FileLineReader& operator=(const FileLineReader& other);
    ~FileLineReader();
    friend std::ostream& operator<<(std::ostream& out, const FileLineReader& reader);
    std::string readLineFromFile();
    bool isOpen() const;
};

// Reads commands from console and stores them
class CommandProcessor {
public:
    // The names of the commands
    static const std::string LOAD_MAP;
    static const std::string VALIDATE_MAP;
    static const std::string ADD_PLAYER;
    static const std::string GAME_START;
    static const std::string REPLAY;
    static const std::string QUIT;

protected:
    std::vector<Command*>* commands;

private:
    virtual std::string readCommand();
    void saveCommand(std::string cmd);

public:
    CommandProcessor();
    CommandProcessor(const CommandProcessor& other);
    CommandProcessor& operator=(const CommandProcessor& other);
    virtual ~CommandProcessor();
    friend std::ostream& operator<<(std::ostream& out, const CommandProcessor& cp);

    Command* getCommand();
    bool validate(Command* cmd, const std::string& currentState);

    // Returns the state a command leads to from a state, empty if the command is not valid there
    std::string getNextState(const std::string& commandName, const std::string& currentState) const;
};

// Adapter that reads commands from a file instead
class FileCommandProcessorAdapter : public CommandProcessor {
private:
    FileLineReader* flr;

    std::string readCommand() override;

public:
    FileCommandProcessorAdapter(std::string filename);
    FileCommandProcessorAdapter(const FileCommandProcessorAdapter& other);
    FileCommandProcessorAdapter& operator=(const FileCommandProcessorAdapter& other);
    ~FileCommandProcessorAdapter() override;
    friend std::ostream& operator<<(std::ostream& out, const FileCommandProcessorAdapter& adapter);
};

#endif