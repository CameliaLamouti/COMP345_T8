#ifndef COMMANDPROCESSING_H
#define COMMANDPROCESSING_H

#include <string>

// Commands of the game
enum class CommandName {
    LOAD_MAP,
    VALIDATE_MAP,
    ADD_PLAYER,
    GAME_START,
    ISSUE_ORDER,
    ISSUE_ORDER_END,
    EXEC_ORDER,
    END_EXEC_ORDER,
    WIN,
    REPLAY,
    QUIT,
    UNKNOWN
};

// Returns the text of a command
std::string commandNameToString(CommandName name);

// Returns the command matching a text, or UNKNOWN
CommandName stringToCommandName(const std::string& text);

#endif