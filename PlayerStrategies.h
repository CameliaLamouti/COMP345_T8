// PlayerStrategies.h
// COMP 345 - Assignment 3 - Part 1: Player strategy pattern
//
// Strategy design pattern: a Player holds a PlayerStrategy and delegates
// issueOrder(), toAttack() and toDefend() to it. Each ConcreteStrategy
// represents one kind of player. The strategy of a player can be replaced
// at any time with Player::setStrategy().
//
// The concrete strategies are skeletons for now: their methods do nothing.

#ifndef PLAYERSTRATEGIES_H
#define PLAYERSTRATEGIES_H

#include <ostream>
#include <string>
#include <vector>

class Player;
class Territory;
class Deck;

// ============================================================
// PlayerStrategy : abstract strategy (NOT a subclass of Player).
// ============================================================
class PlayerStrategy {
public:
    explicit PlayerStrategy(Player* player);               // Creates a strategy for the given player (not owned)
    PlayerStrategy(const PlayerStrategy& other);           // Copy constructor
    PlayerStrategy& operator=(const PlayerStrategy& other);// Assignment operator
    virtual ~PlayerStrategy();                             // Destructor

    virtual bool issueOrder(const std::vector<Player*>& players, Deck* deck) = 0; // Issues one order, false if no more orders this turn
    virtual std::vector<Territory*> toAttack() const = 0;  // Territories the player wants to attack
    virtual std::vector<Territory*> toDefend() const = 0;  // Territories the player wants to defend
    virtual PlayerStrategy* clone() const = 0;             // Returns a copy of this strategy (used when a Player is copied)
    virtual std::string getName() const = 0;               // Returns the name of the kind of player, e.g. "Aggressive"

    Player* getPlayer() const;                             // Returns the player using this strategy
    void setPlayer(Player* player);                        // Sets the player using this strategy

    friend std::ostream& operator<<(std::ostream& out, const PlayerStrategy& strategy); // Stream insertion operator

protected:
    Player* p;   // not owned: the player using this strategy
};

// ============================================================
// HumanPlayerStrategy : decisions come from user interaction.
// ============================================================
class HumanPlayerStrategy : public PlayerStrategy {
public:
    explicit HumanPlayerStrategy(Player* player);
    HumanPlayerStrategy(const HumanPlayerStrategy& other);
    HumanPlayerStrategy& operator=(const HumanPlayerStrategy& other);
    ~HumanPlayerStrategy();

    bool issueOrder(const std::vector<Player*>& players, Deck* deck) override;
    std::vector<Territory*> toAttack() const override;
    std::vector<Territory*> toDefend() const override;
    PlayerStrategy* clone() const override;
    std::string getName() const override;

    friend std::ostream& operator<<(std::ostream& out, const HumanPlayerStrategy& strategy);

private:
    void chooseTargets();                                                            // Asks toDefend() and toAttack() and remembers the answers for this turn
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

// ============================================================
// AggressivePlayerStrategy : focuses on attack.
// ============================================================
class AggressivePlayerStrategy : public PlayerStrategy {
public:
    explicit AggressivePlayerStrategy(Player* player);
    AggressivePlayerStrategy(const AggressivePlayerStrategy& other);
    AggressivePlayerStrategy& operator=(const AggressivePlayerStrategy& other);
    ~AggressivePlayerStrategy();

    bool issueOrder(const std::vector<Player*>& players, Deck* deck) override;
    std::vector<Territory*> toAttack() const override;
    std::vector<Territory*> toDefend() const override;
    PlayerStrategy* clone() const override;
    std::string getName() const override;

    friend std::ostream& operator<<(std::ostream& out, const AggressivePlayerStrategy& strategy);
};

// ============================================================
// BenevolentPlayerStrategy : protects its weakest territories.
// ============================================================
class BenevolentPlayerStrategy : public PlayerStrategy {
public:
    explicit BenevolentPlayerStrategy(Player* player);
    BenevolentPlayerStrategy(const BenevolentPlayerStrategy& other);
    BenevolentPlayerStrategy& operator=(const BenevolentPlayerStrategy& other);
    ~BenevolentPlayerStrategy();

    bool issueOrder(const std::vector<Player*>& players, Deck* deck) override;
    std::vector<Territory*> toAttack() const override;
    std::vector<Territory*> toDefend() const override;
    PlayerStrategy* clone() const override;
    std::string getName() const override;

    friend std::ostream& operator<<(std::ostream& out, const BenevolentPlayerStrategy& strategy);
};

// ============================================================
// NeutralPlayerStrategy : never issues any order.
// ============================================================
class NeutralPlayerStrategy : public PlayerStrategy {
public:
    explicit NeutralPlayerStrategy(Player* player);
    NeutralPlayerStrategy(const NeutralPlayerStrategy& other);
    NeutralPlayerStrategy& operator=(const NeutralPlayerStrategy& other);
    ~NeutralPlayerStrategy();

    bool issueOrder(const std::vector<Player*>& players, Deck* deck) override;
    std::vector<Territory*> toAttack() const override;
    std::vector<Territory*> toDefend() const override;
    PlayerStrategy* clone() const override;
    std::string getName() const override;

    friend std::ostream& operator<<(std::ostream& out, const NeutralPlayerStrategy& strategy);
};

// ============================================================
// CheaterPlayerStrategy : conquers all adjacent territories once per turn.
// ============================================================
class CheaterPlayerStrategy : public PlayerStrategy {
public:
    explicit CheaterPlayerStrategy(Player* player);
    CheaterPlayerStrategy(const CheaterPlayerStrategy& other);
    CheaterPlayerStrategy& operator=(const CheaterPlayerStrategy& other);
    ~CheaterPlayerStrategy();

    bool issueOrder(const std::vector<Player*>& players, Deck* deck) override;
    std::vector<Territory*> toAttack() const override;
    std::vector<Territory*> toDefend() const override;
    PlayerStrategy* clone() const override;
    std::string getName() const override;

    friend std::ostream& operator<<(std::ostream& out, const CheaterPlayerStrategy& strategy);
};

#endif // PLAYERSTRATEGIES_H