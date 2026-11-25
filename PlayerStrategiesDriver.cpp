// PlayerStrategiesDriver.cpp
// COMP 345 - Assignment 3 - Part 1: Player strategy pattern
//
// The human player asks the user to choose targets and a deploy order. The
// computer player uses the Neutral strategy, which automatically ends its turn.
// The human player's strategy is then changed while the game is in progress.

#include <iostream>
#include <vector>

#include "Map.h"
#include "Player.h"
#include "PlayerStrategies.h"

static void issueOneOrder(Player& player, const std::vector<Player*>& players) {
    std::cout << std::endl << player.getName() << " uses the " << *player.getStrategy() << "." << std::endl;
    const bool issued = player.issueOrder(players, nullptr);
    std::cout << player.getName() << (issued ? " issued an order." : " has no more orders this turn.")
              << std::endl;
    std::cout << "Orders so far: " << *player.getOrders() << std::endl;
}

int main() {
    std::cout << "===== Part 1: Player strategy pattern =====" << std::endl;

    Territory home("Human territory", 1);
    Territory computerTerritory("Computer territory", 2);
    home.addNeighbor(&computerTerritory);
    computerTerritory.addNeighbor(&home);

    Player human("Human player");
    Player computer("Computer player");
    human.setStrategy(new HumanPlayerStrategy(&human));
    computer.setStrategy(new NeutralPlayerStrategy(&computer));

    human.addTerritory(&home);
    home.setOwner(&human);
    computer.addTerritory(&computerTerritory);
    computerTerritory.setOwner(&computer);
    human.setPool(3);

    std::vector<Player*> players{ &human, &computer };

    std::cout << std::endl << "--- Different players, different behavior ---" << std::endl;
    std::cout << "The human strategy will ask for your target and deployment choices." << std::endl;
    issueOneOrder(human, players);

    std::cout << std::endl << "The computer strategy acts without asking for input." << std::endl;
    issueOneOrder(computer, players);

    std::cout << std::endl << "--- Changing a strategy during play ---" << std::endl;
    std::cout << human.getName() << " changes from the " << *human.getStrategy()
              << " to the Neutral strategy." << std::endl;
    human.setStrategy(new NeutralPlayerStrategy(&human));
    issueOneOrder(human, players);

    std::cout << std::endl << "===== End of Part 1 demonstration =====" << std::endl;
    return 0;
}
