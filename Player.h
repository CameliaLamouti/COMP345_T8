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

    std::vector<Territory*> toDefend() const;      // Prints the player's territories, asks (console) which ones to defend and returns them
    std::vector<Territory*> toAttack() const;      // Prints the neighboring enemy territories, asks (console) which ones to attack and returns them

    void resetPlannedArmies();                     // Starts a new issuing phase: every territory is planned with the armies it has now and the chosen targets are forgotten

    // Asks the player (through the console) for one order and adds it to the list of orders.
    // Returns true if an order was issued, false if the player has no more orders to issue this turn.
    // The players and the deck are needed by the diplomacy card and by the cards in general.
    bool issueOrder(const std::vector<Player*>& players, Deck* deck);

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

    void chooseTargets();                                                            // Asks toDefend() and toAttack() and remembers the answers for this turn
    int getPlannedArmies(Territory* territory) const;                              // Armies that can still be used from a territory
    bool queueOrder(Order* order);                                                   // Adds an order to the list, deletes it if the list is full
    void printTerritories(const std::string& title, const std::vector<Territory*>& list) const; // Prints a numbered list of territories
    Territory* chooseTerritory(const std::string& title, const std::vector<Territory*>& list) const; // Asks for a territory in a list, nullptr if cancelled
    Player* choosePlayer(const std::vector<Player*>& players) const;                  // Asks for another player, nullptr if cancelled

    bool issueDeployOrder(const std::vector<Territory*>& toDefend);                  // Asks for a deploy order, the only kind allowed while the pool is not empty
    bool issueAdvanceOrder(const std::vector<Territory*>& targets, bool attack);     // Asks for an advance order towards one of the targets
    bool issueCardOrder(const std::vector<Player*>& players, Deck* deck,
                        const std::vector<Territory*>& toDefend,
                        const std::vector<Territory*>& toAttack);                    // Asks for a card to play and the order it creates
    bool issueBombOrder(const std::vector<Territory*>& toAttack);                    // Asks for the target of a bomb card
    bool issueBlockadeOrder(const std::vector<Territory*>& toDefend);                // Asks for the target of a blockade card
    bool issueAirliftOrder();                                                        // Asks for the source, target and armies of an airlift card
    bool issueNegotiateOrder(const std::vector<Player*>& players);                   // Asks for the target of a diplomacy card
};

#endif // PLAYER_H