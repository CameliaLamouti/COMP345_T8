// OrdersDriver.cpp
// COMP 345 - Assignment 2 - Part 4 driver.
//   Verifies every Part 4 requirement:
//   (1) each order is validated before it is executed
//   (2) ownership transfers on Advance conquest
//   (3) one card is awarded per turn when a territory is conquered
//   (4) Negotiate blocks attacks between the two players
//   (5) Blockade transfers ownership to the Neutral player
//   (6) all orders can be issued and executed by the engine
//
// Compile:
//   g++ -std=c++17 OrdersDriver.cpp Orders.cpp Map.cpp Player.cpp Cards.cpp -o orders_part4
// Run:
//   ./orders_part4

#include "Orders.h"
#include "Player.h"
#include "Map.h"
#include "Cards.h"

#include <cstdlib>
#include <iostream>
#include <string>

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

static void section(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
}

// Prints VALID / INVALID for the given order.
static void reportValidate(const std::string& label, const Order& o) {
    std::cout << "  " << label << ": "
              << (o.validate() ? "VALID" : "INVALID") << "\n";
}

// Executes the first Deploy order found in the player's list (if any).
// Returns true if a Deploy order was executed.
static bool executeOneDeploy(Player* player) {
    OrdersList* orders = player->getOrders();
    for (int i = 0; i < orders->size(); i++) {
        if (dynamic_cast<Deploy*>(orders->get(i)) != nullptr) {
            Order* order = orders->get(i);
            bool valid = order->validate();
            order->execute();
            std::cout << "  " << player->getName() << " executes "
                      << order->getDescription()
                      << " -> " << (valid ? "VALID" : "INVALID")
                      << " | " << order->getEffect() << "\n";
            orders->remove(i);   // remove() also deletes the order
            return true;
        }
    }
    return false;
}

// Executes the first order of the player's list, whatever its kind.
// Returns true if an order was executed.
static bool executeOneAny(Player* player) {
    OrdersList* orders = player->getOrders();
    if (orders->size() == 0) {
        return false;
    }
    Order* order = orders->get(0);
    bool valid = order->validate();
    order->execute();
    std::cout << "  " << player->getName() << " executes "
              << order->getDescription()
              << " -> " << (valid ? "VALID" : "INVALID")
              << " | " << order->getEffect() << "\n";
    orders->remove(0);           // remove() also deletes the order
    return true;
}

// Frees a set of territories and players created for one demo.
static void cleanup(Player* a, Player* b,
                    Territory* t1, Territory* t2,
                    Territory* t3, Territory* t4,
                    Deck* deck = nullptr) {
    delete t1;
    delete t2;
    delete t3;
    delete t4;
    delete a;
    delete b;
    delete deck;
}

// ------------------------------------------------------------------
// (1) Every order is validated before being executed
// ------------------------------------------------------------------
static void demoValidation() {
    section("(1) Each order validated before execution");

    Player* alice = new Player("Alice");
    Player* bob   = new Player("Bob");

    Territory* t1 = new Territory("Alaska", 1);
    Territory* t2 = new Territory("Alberta", 2);
    Territory* t3 = new Territory("Ontario", 3);
    Territory* t4 = new Territory("Quebec", 4);

    t1->addNeighbor(t2);
    t2->addNeighbor(t1); t2->addNeighbor(t3);
    t3->addNeighbor(t2); t3->addNeighbor(t4);
    t4->addNeighbor(t3);

    t1->setOwner(alice); alice->addTerritory(t1);
    t2->setOwner(alice); alice->addTerritory(t2);
    t3->setOwner(bob);   bob->addTerritory(t3);
    t4->setOwner(bob);   bob->addTerritory(t4);

    t1->setArmies(10);
    t2->setArmies(10);
    t3->setArmies(5);
    t4->setArmies(5);

    Deploy goodDeploy(alice, t1, 5);
    Deploy badDeploy (bob,   t1, 5);

    Advance goodAdv (alice, t2, t3, 3);
    Advance noAdj   (alice, t1, t4, 3);

    Bomb goodBomb (alice, t3);
    Bomb badBomb  (alice, t1);

    Airlift goodAir (alice, t1, t2, 2);
    Airlift badAir  (alice, t1, t3, 2);

    Blockade goodBlk (alice, t1);
    Blockade badBlk  (alice, t3);

    Negotiate goodNeg (alice, bob);
    Negotiate selfNeg (alice, alice);

    reportValidate("Deploy good",    goodDeploy);
    reportValidate("Deploy bad",     badDeploy);
    reportValidate("Advance good",   goodAdv);
    reportValidate("Advance no-adj", noAdj);
    reportValidate("Bomb good",      goodBomb);
    reportValidate("Bomb own",       badBomb);
    reportValidate("Airlift good",   goodAir);
    reportValidate("Airlift bad",    badAir);
    reportValidate("Blockade good",  goodBlk);
    reportValidate("Blockade bad",   badBlk);
    reportValidate("Negotiate good", goodNeg);
    reportValidate("Negotiate self", selfNeg);

    cleanup(alice, bob, t1, t2, t3, t4);
}

// ------------------------------------------------------------------
// (2) Ownership transfer on Advance conquest
// ------------------------------------------------------------------
static void demoConquest() {
    section("(2) Advance conquest transfers ownership");

    Player* alice = new Player("Alice");
    Player* bob   = new Player("Bob");

    Territory* t1 = new Territory("Alaska", 1);
    Territory* t2 = new Territory("Alberta", 2);
    Territory* t3 = new Territory("Ontario", 3);
    Territory* t4 = new Territory("Quebec", 4);

    t1->addNeighbor(t2);
    t2->addNeighbor(t1); t2->addNeighbor(t3);
    t3->addNeighbor(t2); t3->addNeighbor(t4);
    t4->addNeighbor(t3);

    t1->setOwner(alice); alice->addTerritory(t1);
    t2->setOwner(alice); alice->addTerritory(t2);
    t3->setOwner(bob);   bob->addTerritory(t3);
    t4->setOwner(bob);   bob->addTerritory(t4);

    t2->setArmies(50);
    t3->setArmies(0);   // guaranteed capture

    std::cout << "Before: t3 owner = "
              << (t3->getOwner() ? t3->getOwner()->getName() : "none")
              << ", armies = " << t3->getArmies() << "\n";

    Advance adv(alice, t2, t3, 10);
    adv.execute();

    std::cout << "Effect: " << adv.getEffect() << "\n";
    std::cout << "After:  t3 owner = "
              << (t3->getOwner() ? t3->getOwner()->getName() : "none")
              << ", armies = " << t3->getArmies() << "\n";
    std::cout << "didConquer() = " << (adv.didConquer() ? "true" : "false") << "\n";

    cleanup(alice, bob, t1, t2, t3, t4);
}

// ------------------------------------------------------------------
// (3) Exactly one card awarded per turn on conquest
// ------------------------------------------------------------------
static void demoCardAward() {
    section("(3) One card awarded on conquest");

    Player* alice = new Player("Alice");
    Player* bob   = new Player("Bob");
    Deck* deck    = new Deck();

    Territory* t1 = new Territory("Alaska", 1);
    Territory* t2 = new Territory("Alberta", 2);
    Territory* t3 = new Territory("Ontario", 3);
    Territory* t4 = new Territory("Quebec", 4);

    t1->addNeighbor(t2);
    t2->addNeighbor(t1); t2->addNeighbor(t3);
    t3->addNeighbor(t2); t3->addNeighbor(t4);
    t4->addNeighbor(t3);

    t1->setOwner(alice); alice->addTerritory(t1);
    t2->setOwner(alice); alice->addTerritory(t2);
    t3->setOwner(bob);   bob->addTerritory(t3);
    t4->setOwner(bob);   bob->addTerritory(t4);

    std::cout << "Alice hand before: " << *alice->getHand() << "\n";
    std::cout << "Deck before:       " << deck->size() << " cards\n";

    t2->setArmies(50);
    t3->setArmies(0);

    Advance adv(alice, t2, t3, 10);
    adv.execute();

    // The engine (not the order) awards the card, so that a player
    // cannot be given more than one per turn in the game loop.
    if (adv.didConquer()) {
        Card* c = deck->draw(alice->getHand());
        std::cout << "Awarded card: " << (c ? c->getTypeName() : "none") << "\n";
    }

    std::cout << "Alice hand after:  " << *alice->getHand() << "\n";
    std::cout << "Deck after:        " << deck->size() << " cards\n";

    cleanup(alice, bob, t1, t2, t3, t4, deck);
}

// ------------------------------------------------------------------
// (4) Negotiate blocks attacks in both directions
// ------------------------------------------------------------------
static void demoNegotiate() {
    section("(4) Negotiate blocks attacks");

    Player* alice = new Player("Alice");
    Player* bob   = new Player("Bob");

    Territory* t1 = new Territory("Alaska", 1);
    Territory* t2 = new Territory("Alberta", 2);
    Territory* t3 = new Territory("Ontario", 3);
    Territory* t4 = new Territory("Quebec", 4);

    t1->addNeighbor(t2);
    t2->addNeighbor(t1); t2->addNeighbor(t3);
    t3->addNeighbor(t2); t3->addNeighbor(t4);
    t4->addNeighbor(t3);

    t1->setOwner(alice); alice->addTerritory(t1);
    t2->setOwner(alice); alice->addTerritory(t2);
    t3->setOwner(bob);   bob->addTerritory(t3);
    t4->setOwner(bob);   bob->addTerritory(t4);

    t2->setArmies(50);
    t3->setArmies(5);
    t4->setArmies(50);

    Advance beforeNeg(alice, t2, t3, 5);
    std::cout << "Before negotiation, Alice->Bob valid? "
              << (beforeNeg.validate() ? "yes" : "no") << "\n";

    Negotiate neg(alice, bob);
    neg.execute();
    std::cout << "Negotiate effect: " << neg.getEffect() << "\n";

    Advance afterNeg(alice, t2, t3, 5);
    std::cout << "After negotiation,  Alice->Bob valid? "
              << (afterNeg.validate() ? "yes" : "no") << "\n";

    // Reverse direction: Bob attacks Alice's t2 from t3 (adjacent).
    Advance bobOnAlice(bob, t3, t2, 5);
    std::cout << "Bob->Alice attack valid?           "
              << (bobOnAlice.validate() ? "yes" : "no") << "\n";

    cleanup(alice, bob, t1, t2, t3, t4);
}

// ------------------------------------------------------------------
// (5) Blockade doubles armies and transfers to Neutral
// ------------------------------------------------------------------
static void demoBlockade() {
    section("(5) Blockade -> Neutral player");

    Player* alice = new Player("Alice");
    Player* bob   = new Player("Bob");

    Territory* t1 = new Territory("Alaska", 1);
    Territory* t2 = new Territory("Alberta", 2);
    Territory* t3 = new Territory("Ontario", 3);
    Territory* t4 = new Territory("Quebec", 4);

    t1->setOwner(alice); alice->addTerritory(t1);
    t2->setOwner(alice); alice->addTerritory(t2);
    t3->setOwner(bob);   bob->addTerritory(t3);
    t4->setOwner(bob);   bob->addTerritory(t4);

    t1->setArmies(7);
    std::cout << "Before: t1 armies = " << t1->getArmies()
              << ", owner = " << t1->getOwner()->getName() << "\n";

    Blockade blk(alice, t1);
    blk.execute();

    std::cout << "Effect: " << blk.getEffect() << "\n";
    std::cout << "After:  t1 armies = " << t1->getArmies()
              << ", owner = "
              << (t1->getOwner() ? t1->getOwner()->getName() : "Neutral")
              << "\n";

    cleanup(alice, bob, t1, t2, t3, t4);
}

// ------------------------------------------------------------------
// (6) All the orders can be issued by a player and executed similar 
// to the game engine itself.
// ------------------------------------------------------------------
static void demoIssueAndExecute() {
    section("(6) Orders issued by a player and executed engine-style");

    Player* alice = new Player("Alice");
    Player* bob   = new Player("Bob");
    Deck* deck    = new Deck();

    Territory* t1 = new Territory("Alaska", 1);
    Territory* t2 = new Territory("Alberta", 2);
    Territory* t3 = new Territory("Ontario", 3);
    Territory* t4 = new Territory("Quebec", 4);

    t1->addNeighbor(t2);
    t2->addNeighbor(t1); t2->addNeighbor(t3);
    t3->addNeighbor(t2); t3->addNeighbor(t4);
    t4->addNeighbor(t3);

    t1->setOwner(alice); alice->addTerritory(t1);
    t2->setOwner(alice); alice->addTerritory(t2);
    t3->setOwner(bob);   bob->addTerritory(t3);
    t4->setOwner(bob);   bob->addTerritory(t4);

    t1->setArmies(20);
    t2->setArmies(10);
    t3->setArmies(3);
    t4->setArmies(2);

    // Give the players the cards they need to try the special orders.
    alice->getHand()->addCard(new Card(CardType::Bomb));
    alice->getHand()->addCard(new Card(CardType::Airlift));
    alice->getHand()->addCard(new Card(CardType::Blockade));
    alice->getHand()->addCard(new Card(CardType::Diplomacy));

    std::vector<Player*> players;
    players.push_back(alice);
    players.push_back(bob);

    // -------- Issue phase: the players issue their orders themselves --------
    std::cout << "Issue phase (each player's issueOrder() prompts the console).\n"
                 "Alice has 5 armies in her pool; Bob has 2.\n"
                 "End every turn with choice 0.\n";
    alice->setPool(5);
    bob->setPool(2);

    alice->resetPlannedArmies();
    while (alice->issueOrder(players, deck)) {
        // Player::issueOrder keeps asking until the player chooses 0.
    }

    bob->resetPlannedArmies();
    while (bob->issueOrder(players, deck)) {
    }

    std::cout << "\nAlice's OrdersList:\n" << *alice->getOrders() << "\n";
    std::cout << "Bob's OrdersList:\n"   << *bob->getOrders()   << "\n";

    // -------- Execute phase: mirrors GameEngine::executeOrdersPhase() --------
    std::cout << "\nExecute phase (deploy orders first, then the rest):\n";

    // Pass 1: every Deploy order, one player at a time.
    bool anyDeploy = true;
    while (anyDeploy) {
        anyDeploy = false;
        for (size_t i = 0; i < players.size(); i++) {
            if (executeOneDeploy(players[i])) {
                anyDeploy = true;
            }
        }
    }

    // Pass 2: the remaining orders, round-robin.
    bool anyLeft = true;
    while (anyLeft) {
        anyLeft = false;
        for (size_t i = 0; i < players.size(); i++) {
            if (executeOneAny(players[i])) {
                anyLeft = true;
            }
        }
    }

    std::cout << "\nBoth lists are empty now.\n";

    cleanup(alice, bob, t1, t2, t3, t4, deck);   
}


int main() {

    std::cout << "=== COMP 345 - Assignment 2 - Part 4 demo ===\n";

    demoValidation();
    demoConquest();
    demoCardAward();
    demoNegotiate();
    demoBlockade();
    demoIssueAndExecute();

    return 0;
}