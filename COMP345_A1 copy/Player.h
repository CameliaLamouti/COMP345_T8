// Player.h
// COMP 345 - Assignment 1 - Part 2: Player

#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <string>
#include <vector>

#include "Cards.h"
#include "Map.h"
#include "Orders.h"

/**
 * Player owns a hand of cards, a list of orders
 * and a collection of territories from the Map.
 */
class Player {
public:
    explicit Player(const std::string& name);      // Creates a player with no territories, an empty hand and an empty orders list
    Player(const Player& other);                   // Copy constructor
    Player& operator=(const Player& other);        // Assignment operator
    ~Player();                                     // Destructor

    std::string getName() const;                   // Returns the player's name
    void addTerritory(Territory* territory);       // Adds a territory to the ones the player owns
    void addAttackTarget(Territory* territory);    // Adds a territory to the ones returned by toAttack()
    std::vector<Territory*> getTerritories() const;// Returns the territories the player owns
    Hand* getHand() const;                         // Returns the player's hand of cards
    OrdersList* getOrders() const;                 // Returns the player's list of orders

    std::vector<Territory*> toDefend() const;      // Returns the territories to be defended
    std::vector<Territory*> toAttack() const;      // Returns the territories to be attacked
    bool issueOrder(const std::string& orderType); // Creates an order and adds it to the player's list of orders

    friend std::ostream& operator<<(std::ostream& out, const Player& player); // Stream insertion operator

private:
    std::string* name;
    std::vector<Territory*>* territories;     // not owned: territories belong to the Map
    std::vector<Territory*>* attackTargets;   // not owned
    Hand* hand;                               // owned
    OrdersList* orders;                       // owned
};

#endif // PLAYER_H
