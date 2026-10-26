// OrdersDriver.cpp
// COMP 345 - Assignment 1 - Part 3 driver.
// Creates one of every order kind, puts them in an OrdersList,
// demonstrates move(), remove(), validate(), execute(), and operator<<.
// validate() still does its basic checks
// (e.g. armies > 0) and execute() only records a placeholder effect.

// How to use:
// 1) Compile: g++ -std=c++17 Orders.cpp OrdersDriver.cpp Map.cpp Player.cpp Cards.cpp -o orders_driver
// 2) Run:     ./orders_driver

#include "Orders.h"
#include "Player.h"
#include "Map.h"

#include <iostream>

int main() {
    std::cout << "=== Part 3: OrdersList demo ===\n\n";

    // Create Player/Territories
    Player* p1 = new Player("Bob");
    Player* p2 = new Player("Steve");

    Territory* t1 = new Territory("Alaska",1);
    Territory* t2 = new Territory("Northwest Territories",2);
    Territory* t3 = new Territory("Alberta",3);
    Territory* t4 = new Territory("Ontario",4);

    // Set up the map 
    t1->addNeighbor(t2);
    t1->addNeighbor(t3);
    t2->addNeighbor(t1);
    t3->addNeighbor(t1);
    t3->addNeighbor(t4);
    t4->addNeighbor(t3);

    // Set up ownership
    t1->setOwner(p1); // p1 owns Alaska
    t2->setOwner(p1);
    t3->setOwner(p1);
    t4->setOwner(p2); // p2 owns Ontario

    // Each player also keeps the list of territories it owns
    p1->addTerritory(t1);
    p1->addTerritory(t2);
    p1->addTerritory(t3);
    p2->addTerritory(t4);

    // Set armies on territory
    t1->setArmies(15);
    t4->setArmies(8);  // so the Bomb order has armies to halve

    // Create the list
    OrdersList* list = new OrdersList();

    // One of each order kind 

    // Deploy: p1 deploys 5 armies to t1 (which p1 owns).
    list->add(new Deploy   (p1, t1, 5));
    // Advance: p1 moves 3 armies from t1 to t2 (adjacent and both owned by p1).
    list->add(new Advance  (p1, t1, t2, 3));
    // Bomb: p1 bombs t4 (owned by p2, and adjacent to t3 which p1 owns).
    list->add(new Bomb     (p1, t4));
    // Airlift: p1 airlifts 4 armies from t1 to t3 (no adjacency required).
    list->add(new Airlift  (p1, t1, t3, 4));
    // Blockade: p1 blockades t1 (owned by p1).
    list->add(new Blockade (p1, t1));
    // Negotiate: p1 negotiates with p2.
    list->add(new Negotiate(p1, p2));

    // An intentionally invalid order (armies = 0) to show invalid path
    list->add(new Deploy   (p1, t1, 0));

    std::cout << "--- Initial list (7 orders) ---\n";
    std::cout << *list << "\n\n";

    std::cout << "--- validate() each order ---\n";
    for (int i = 0; i < list->size(); ++i)
        std::cout << "  [" << i << "] valid? "
                  << (list->get(i)->validate() ? "yes" : "no") << "\n";
    std::cout << "\n";

    std::cout << "--- execute() each order ---\n";
    for (int i = 0; i < list->size(); ++i)
        list->get(i)->execute();
    std::cout << *list << "\n\n";

    std::cout << "--- move(0, 3) : move Deploy from front to index 3 ---\n";
    list->move(0, 3);
    std::cout << *list << "\n\n";

    std::cout << "--- remove(2) : delete the order at index 2 ---\n";
    list->remove(2);
    std::cout << *list << "\n\n";

    std::cout << "--- copy constructor : deep copy survives delete of original ---\n";
    OrdersList copy(*list);
    delete list;                    // original gone -- copy must be intact
    std::cout << copy << "\n";

    // Clean up dynamic memory for Players/Territories.
    delete t1;
    delete t2;
    delete t3;
    delete t4;
    delete p1;
    delete p2;

    return 0;
}