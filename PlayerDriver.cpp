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
    for (int i = 0; i < 4; ++i) alice->addTerritory(map[i]);
    for (int i = 4; i < 6; ++i) bob->addTerritory(map[i]);
 
    // Alice may attack Bob's territories.
    alice->addAttackTarget(map[4]);
    alice->addAttackTarget(map[5]);
 
    // Players own a hand of cards.
    alice->getHand()->addCard(new Card("bomb"));
    alice->getHand()->addCard(new Card("airlift"));
 
    std::cout << "=== toDefend() and toAttack() ===" << std::endl;
    printTerritories("Alice toDefend", alice->toDefend());
    printTerritories("Alice toAttack", alice->toAttack());
 
    std::cout << "\n=== issueOrder() ===" << std::endl;
    std::cout << "Issue deploy order" << std::endl;
    alice->issueOrder("deploy");
    std::cout << "Issue advance order" << std::endl;
    alice->issueOrder("advance");
    std::cout << "Issue teleport order (invalid)" << std::endl;
    alice->issueOrder("teleport");
    std::cout << *alice << std::endl;
 
    std::cout << "\n=== Copy constructor and assignment operator ===" << std::endl;
    Player copy(*alice);
    copy.issueOrder("deploy");
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