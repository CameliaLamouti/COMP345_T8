#include "CommandProcessing.h"
#include <string>

// Returns the text of a command
std::string commandNameToString(CommandName name) {
    switch (name) {
        case CommandName::LOAD_MAP: return "loadmap";
        case CommandName::VALIDATE_MAP: return "validatemap";
        case CommandName::ADD_PLAYER: return "addplayer";
        case CommandName::GAME_START: return "gamestart";
        case CommandName::ISSUE_ORDER: return "issueorder";
        case CommandName::ISSUE_ORDER_END: return "issueorderend";
        case CommandName::EXEC_ORDER: return "execorder";
        case CommandName::END_EXEC_ORDER: return "endexecorder";
        case CommandName::WIN: return "win";
        case CommandName::REPLAY: return "replay";
        case CommandName::QUIT: return "quit";
        case CommandName::UNKNOWN: return "unknown";
    }
    return "unknown";
}

// Returns the command matching a text, or UNKNOWN
CommandName stringToCommandName(const std::string& text) {
    for (CommandName name : {
        CommandName::LOAD_MAP,
        CommandName::VALIDATE_MAP,
        CommandName::ADD_PLAYER,
        CommandName::GAME_START,
        CommandName::ISSUE_ORDER,
        CommandName::ISSUE_ORDER_END,
        CommandName::EXEC_ORDER,
        CommandName::END_EXEC_ORDER,
        CommandName::WIN,
        CommandName::REPLAY,
        CommandName::QUIT
    }) {
        if (text == commandNameToString(name)) {
            return name;
        }
    }
    return CommandName::UNKNOWN;
}