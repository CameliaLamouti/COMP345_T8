#include <iostream>
#include <vector>

#include "Player.h"

//print a list of territories
static void printTerritories(const std::string& title, const std::vector<Territory*>& list){
    std::cout << title << ":";
    for (Territory* t :list) {
        std::cout << " [" << *t << "]";
    }
    std::cout << std::endl;    
}

//player class features
int main() {
    // Territories normally belong to the Map; here the driver owns them.
    std::vector<Territory*> map;
    const char* names[] = {"Quebec", "Ontario", "Alberta", "Yukon", "Manitoba", "Nunavut"};
    for (int i = 0; i < 6; ++i) {
        map.push_back(new Territory(names[i], i + 1));
    }
 
    Player* alice = new Player("Alice");
    Player* bob = new Player("Bob");
 
    // Alice owns the first four territories, Bob the last two.
    for (int i = 0; i < 4; ++i) {
        alice->addTerritory(map[i]);
        map[i]->setOwner(alice);
        map[i]->setArmies(3);
    }
    for (int i = 4; i < 6; ++i) {
        bob->addTerritory(map[i]);
        map[i]->setOwner(bob);
        map[i]->setArmies(2);
    }
 
    // Alice may attack Bob's territories connected to her border.
    map[0]->addNeighbor(map[4]);
    map[4]->addNeighbor(map[0]);
    map[3]->addNeighbor(map[5]);
    map[5]->addNeighbor(map[3]);
 
    // Players own a hand of cards.
    alice->getHand()->addCard(new Card("bomb"));
    alice->getHand()->addCard(new Card("airlift"));
 
    std::cout << "=== toDefend() and toAttack() ===" << std::endl;
    printTerritories("Alice toDefend", alice->toDefend());
    printTerritories("Alice toAttack", alice->toAttack());
 
    std::cout << "\n=== issueOrder() ===" << std::endl;
    std::vector<Player*> players = {alice, bob};
    Deck deck;
    alice->setPool(3);
    alice->resetPlannedArmies();
    std::cout << "Issue orders interactively; end the turn with choice 0." << std::endl;
    while (alice->issueOrder(players, &deck)) {
    }
    std::cout << *alice << std::endl;

    std::cout << "\n=== Copy constructor and assignment operator ===" << std::endl;
    Player copy(*alice);
    Order* demonstrationOrder = new Deploy(&copy, map[0], 1);
    if (!copy.getOrders()->add(demonstrationOrder)) {
        delete demonstrationOrder;
        std::cout << "Could not add a demonstration order to the copy." << std::endl;
    }
    std::cout << "Original orders: " << *alice->getOrders() << std::endl;
    std::cout << "Copy orders:     " << *copy.getOrders() << std::endl;
 
    Player assigned("Temp");
    assigned = *bob;
    std::cout << assigned << std::endl;
 
    delete alice;
    delete bob;
    for (Territory* t : map) delete t;
 
    return 0;
}