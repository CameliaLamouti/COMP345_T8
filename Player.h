// Player.h
// COMP 345 - Assignment 2: Player

#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Cards.h"
#include "Map.h"
#include "Orders.h"

class Territory;
class PlayerStrategy;

/**
 * Player owns a hand of cards, a list of orders, a pool of armies
 * and a collection of territories from the Map.
 * During the issuing phase it also remembers which territories the player
 * chose to defend and to attack this turn.
 */
class Player {
public:
    explicit Player(const std::string& name);      // Creates a player with no territories, an empty hand, an empty orders list and an empty pool
    Player(const Player& other);                   // Copy constructor
    Player& operator=(const Player& other);        // Assignment operator
    ~Player();                                     // Destructor

    std::string getName() const;                   // Returns the player's name
    void addTerritory(Territory* territory);       // Adds a territory to the ones the player owns
    void clearTerritories();                       // Forgets all the territories of the player (the Map keeps owning them)
    std::vector<Territory*> getTerritories() const;// Returns the territories the player owns
    Hand* getHand() const;                         // Returns the player's hand of cards
    OrdersList* getOrders() const;                 // Returns the player's list of orders
    int getPool() const;                           // Returns the number of armies waiting to be deployed
    void setPool(int armies);                      // Sets the number of armies waiting to be deployed

    void setStrategy(PlayerStrategy* newStrategy); // Replaces the strategy of the player (can be done at any time); the player takes ownership of it
    PlayerStrategy* getStrategy() const;           // Returns the strategy currently used by the player

    // Strategy pattern: these three methods only delegate to the player's strategy.
    std::vector<Territory*> toDefend() const;      // Delegates to the strategy: territories to defend
    std::vector<Territory*> toAttack() const;      // Delegates to the strategy: territories to attack
    // Delegates to the strategy: issues one order. Returns true if an order was issued,
    // false if the player has no more orders to issue this turn.
    // The players and the deck are needed by the diplomacy card and by the cards in general.
    bool issueOrder(const std::vector<Player*>& players, Deck* deck);

    void resetPlannedArmies();                     // Starts a new issuing phase: every territory is planned with the armies it has now and the chosen targets are forgotten

    void addNegotiation(Player* other);            // Records that this player cannot attack 'other' this turn
    bool isNegotiatingWith(Player* other) const;   // Tells whether attacks against 'other' are blocked
    void clearNegotiations();                      // Removes all negotiations (called at the start of a new turn)

    friend std::ostream& operator<<(std::ostream& out, const Player& player); // Stream insertion operator

private:
    std::string* name;
    std::vector<Territory*>* territories;     // not owned: territories belong to the Map
    std::vector<Territory*>* defendTargets;   // not owned: territories chosen with toDefend() this turn
    std::vector<Territory*>* attackTargets;   // not owned: territories chosen with toAttack() this turn
    bool* targetsChosen;                      // owned: tells whether toDefend() and toAttack() were already asked this turn
    std::map<Territory*, int>* plannedArmies; // owned: armies a territory will have once the orders issued so far are executed
    Hand* hand;                               // owned
    OrdersList* orders;                       // owned
    int* pool;                                // owned: armies not yet deployed
    std::vector<Player*>* negotiatedWith;     // owned: players this player is negotiating with this turn
    PlayerStrategy* strategy;                 // owned: decides how the player issues orders and chooses targets

    int getPlannedArmies(Territory* territory) const;// Armies that can still be used from a territory
    bool queueOrder(Order* order);                   // Adds an order to the list, deletes it if the list is full

    friend class HumanPlayerStrategy;         // the human strategy works directly on the player's turn state (targets, planned armies, pool, hand, orders)
};

#endif // PLAYER_H