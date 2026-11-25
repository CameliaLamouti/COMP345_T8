// PlayerStrategies.cpp
// COMP 345 - Assignment 3 - Part 1: Player strategy pattern
// Implementation of the classes declared in PlayerStrategies.h.
// HumanPlayerStrategy asks the user (through the console) what to do.
// The other concrete strategies are skeletons: issueOrder(), toAttack() and toDefend() do nothing yet.

#include "PlayerStrategies.h"

#include "Player.h"

#include <algorithm>
#include <sstream>

// Armies a player receives in the pool when playing a reinforcement card
static const int REINFORCEMENT_CARD_ARMIES = 5;

/**
 * Asks for a whole number between minValue and maxValue (both included) until
 * the answer is valid. Returns false if the input ended before an answer was read.
 */
static bool askNumber(const std::string& prompt, int minValue, int maxValue, int& value) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return false;
        }
        std::istringstream input(line);
        int number = 0;
        if ((input >> number) && (input >> std::ws).eof() && number >= minValue && number <= maxValue) {
            value = number;
            return true;
        }
        std::cout << "  Please enter a number between " << minValue << " and " << maxValue << "." << std::endl;
    }
}

/**
 * Asks for a list of numbers between 1 and count, written on one line and
 * separated by spaces (0 or an empty line means none). Repeats until the answer
 * is valid. Returns false if the input ended before an answer was read.
 */
static bool askSelection(const std::string& prompt, int count, std::vector<int>& selection) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return false;
        }

        std::istringstream input(line);
        std::vector<int> numbers;
        std::string token;
        bool valid = true;
        while (input >> token) {
            std::istringstream tokenStream(token);
            int number = 0;
            if (!(tokenStream >> number) || !(tokenStream >> std::ws).eof() || number < 0 || number > count) {
                valid = false;
                break;
            }
            numbers.push_back(number);
        }

        if (valid) {
            selection.clear();
            for (int number : numbers) {
                if (number > 0 && std::find(selection.begin(), selection.end(), number) == selection.end()) {
                    selection.push_back(number);
                }
            }
            return true;
        }
        std::cout << "  Please enter numbers between 1 and " << count << ", separated by spaces (0 for none)." << std::endl;
    }
}

/**
 * Returns the name of the owner of a territory ("nobody" if it has none).
 */
static std::string ownerName(const Territory* territory) {
    Player* owner = territory->getOwner();
    return owner == nullptr ? "nobody" : owner->getName();
}

// ============================================================
// PlayerStrategy
// ============================================================

/**
 * Creates a strategy for the given player (the player is not owned).
 */
PlayerStrategy::PlayerStrategy(Player* player) : p(player) {}

/**
 * Copy constructor: the copy is used by the same player.
 */
PlayerStrategy::PlayerStrategy(const PlayerStrategy& other) : p(other.p) {}

/**
 * Assignment operator.
 */
PlayerStrategy& PlayerStrategy::operator=(const PlayerStrategy& other) {
    if (this != &other) {
        p = other.p;
    }
    return *this;
}

/**
 * Destructor: the player is not owned, so nothing is deleted.
 */
PlayerStrategy::~PlayerStrategy() {}

/**
 * Returns the player using this strategy.
 */
Player* PlayerStrategy::getPlayer() const {
    return p;
}

/**
 * Sets the player using this strategy.
 */
void PlayerStrategy::setPlayer(Player* player) {
    p = player;
}

/**
 * Stream insertion operator: prints the kind of player, e.g. "Aggressive strategy".
 */
std::ostream& operator<<(std::ostream& out, const PlayerStrategy& strategy) {
    out << strategy.getName() << " strategy";
    return out;
}

// ============================================================
// HumanPlayerStrategy
// ============================================================

/**
 * Creates a human strategy for the given player.
 */
HumanPlayerStrategy::HumanPlayerStrategy(Player* player) : PlayerStrategy(player) {}

/**
 * Copy constructor.
 */
HumanPlayerStrategy::HumanPlayerStrategy(const HumanPlayerStrategy& other) : PlayerStrategy(other) {}

/**
 * Assignment operator.
 */
HumanPlayerStrategy& HumanPlayerStrategy::operator=(const HumanPlayerStrategy& other) {
    PlayerStrategy::operator=(other);
    return *this;
}

/**
 * Destructor.
 */
HumanPlayerStrategy::~HumanPlayerStrategy() {}

/**
 * Asks the player what order to issue. While the pool still has armies only a
 * deploy order is possible. Once the pool is empty the player can advance
 * armies to defend (targets chosen with toDefend()), advance armies to attack
 * (targets chosen with toAttack()), play a card, choose the targets again, or
 * end the turn. toDefend() and toAttack() are asked once per turn, the first
 * time the player is asked for an order. Orders are not validated
 * here, only when they are executed.
 * Returns true if an order was issued, false if the player has no more orders this turn.
 */
bool HumanPlayerStrategy::issueOrder(const std::vector<Player*>& players, Deck* deck) {
    std::cout << std::endl << "Armies left to deploy: " << *p->pool << std::endl
              << "Orders issued so far: " << p->orders->size() << std::endl;

    if (!*p->targetsChosen) {
        chooseTargets();
    }

    if (*p->pool > 0) {
        std::cout << std::endl << "You still have armies to deploy, so only a deploy order is allowed." << std::endl;
        // deploying needs at least one territory to defend
        while (p->defendTargets->empty()) {
            if (std::cin.eof()) {
                return false;
            }
            std::cout << std::endl << "You must choose at least one territory to defend to deploy your armies." << std::endl;
            *p->defendTargets = toDefend();
        }
        return issueDeployOrder(*p->defendTargets);
    }

    while (true) {
        if (p->orders->size() >= p->orders->capacity()) {
            std::cout << std::endl << "Your list of orders is full, your turn is over." << std::endl;
            return false;
        }

        std::cout << std::endl << "What do you want to do?" << std::endl
                  << "  1. Advance armies to defend a territory" << std::endl
                  << "  2. Advance armies to attack a territory" << std::endl
                  << "  3. Play a card" << std::endl
                  << "  4. Choose again the territories to defend and attack" << std::endl
                  << "  0. End my turn (no more orders)" << std::endl;
        int choice = 0;
        if (!askNumber("Your choice: ", 0, 4, choice) || choice == 0) {
            return false;
        }

        bool issued = false;
        if (choice == 1) {
            issued = issueAdvanceOrder(*p->defendTargets, false);
        } else if (choice == 2) {
            issued = issueAdvanceOrder(*p->attackTargets, true);
        } else if (choice == 3) {
            issued = issueCardOrder(players, deck, *p->defendTargets, *p->attackTargets);
        } else {
            chooseTargets();
        }
        if (issued) {
            // a reinforcement card gives new armies, which must be deployed first
            return true;
        }
    }
}

/**
 * Prints all the territories next to one of the player's territories and owned
 * by somebody else, asks (through the console) which ones to attack and returns
 * them. The answer is a list of numbers separated by spaces, 0 or an empty line
 * means none.
 */
std::vector<Territory*> HumanPlayerStrategy::toAttack() const {
    std::vector<Territory*> neighbors;
    for (Territory* territory : *p->territories) {
        for (Territory* neighbor : territory->getNeighbors()) {
            if (neighbor->getOwner() != p
                && std::find(neighbors.begin(), neighbors.end(), neighbor) == neighbors.end()) {
                neighbors.push_back(neighbor);
            }
        }
    }

    std::vector<Territory*> result;
    printTerritories("Neighboring territories you could attack", neighbors);

    std::vector<int> selection;
    if (neighbors.empty()
        || !askSelection("Territories to attack (numbers separated by spaces, 0 for none): ",
                         static_cast<int>(neighbors.size()), selection)) {
        return result;
    }
    for (int number : selection) {
        result.push_back(neighbors[number - 1]);
    }
    return result;
}

/**
 * Prints all the territories the player owns, asks (through the console) which
 * ones to defend and returns them. The answer is a list of numbers separated by
 * spaces, 0 or an empty line means none.
 */
std::vector<Territory*> HumanPlayerStrategy::toDefend() const {
    std::vector<Territory*> result;
    printTerritories("Your territories", *p->territories);

    std::vector<int> selection;
    if (p->territories->empty()
        || !askSelection("Territories to defend (numbers separated by spaces, 0 for none): ",
                         static_cast<int>(p->territories->size()), selection)) {
        return result;
    }
    for (int number : selection) {
        result.push_back((*p->territories)[number - 1]);
    }
    return result;
}

/**
 * Asks toDefend() and toAttack() and remembers the answers for the rest of the turn.
 */
void HumanPlayerStrategy::chooseTargets() {
    *p->defendTargets = toDefend();
    *p->attackTargets = toAttack();
    *p->targetsChosen = true;
}

/**
 * Prints a numbered list of territories with their owner and armies.
 */
void HumanPlayerStrategy::printTerritories(const std::string& title, const std::vector<Territory*>& list) const {
    std::cout << std::endl << title << ":" << std::endl;
    if (list.empty()) {
        std::cout << "  (none)" << std::endl;
        return;
    }
    for (size_t i = 0; i < list.size(); i++) {
        Territory* territory = list[i];
        std::cout << "  " << (i + 1) << ". " << territory->getName()
                  << " - owner: " << ownerName(territory)
                  << ", armies: " << territory->getArmies();
        if (territory->getOwner() == p) {
            std::cout << " (available for orders: " << p->getPlannedArmies(territory) << ")";
        }
        std::cout << std::endl;
    }
}

/**
 * Prints a list of territories and asks for one of them.
 * Returns nullptr if the player cancels or the input ends.
 */
Territory* HumanPlayerStrategy::chooseTerritory(const std::string& title, const std::vector<Territory*>& list) const {
    printTerritories(title, list);
    int choice = 0;
    if (!askNumber("Your choice (1-" + std::to_string(list.size()) + ", 0 to cancel): ",
                   0, static_cast<int>(list.size()), choice) || choice == 0) {
        return nullptr;
    }
    return list[choice - 1];
}

/**
 * Asks for one of the other players. Returns nullptr if the player cancels
 * or if there is no other player.
 */
Player* HumanPlayerStrategy::choosePlayer(const std::vector<Player*>& players) const {
    std::vector<Player*> others;
    for (Player* candidate : players) {
        if (candidate != p) {
            others.push_back(candidate);
        }
    }
    if (others.empty()) {
        std::cout << std::endl << "There is no other player." << std::endl;
        return nullptr;
    }

    std::cout << std::endl << "Players:" << std::endl;
    for (size_t i = 0; i < others.size(); i++) {
        std::cout << "  " << (i + 1) << ". " << others[i]->getName() << std::endl;
    }
    int choice = 0;
    if (!askNumber("Your choice (1-" + std::to_string(others.size()) + ", 0 to cancel): ",
                   0, static_cast<int>(others.size()), choice) || choice == 0) {
        return nullptr;
    }
    return others[choice - 1];
}

/**
 * Asks for a deploy order: a territory to defend and a number of armies from
 * the pool. The armies leave the pool as soon as the order is issued.
 * Returns false if no order was issued.
 */
bool HumanPlayerStrategy::issueDeployOrder(const std::vector<Territory*>& toDefend) {
    if (toDefend.empty()) {
        std::cout << std::endl << "You have no territory to deploy on." << std::endl;
        return false;
    }

    printTerritories("Territories you chose to defend", toDefend);
    int choice = 0;
    int armies = 0;
    if (!askNumber("Territory to deploy on (1-" + std::to_string(toDefend.size()) + "): ",
                   1, static_cast<int>(toDefend.size()), choice)) {
        return false;
    }
    if (!askNumber("Armies to deploy (1-" + std::to_string(*p->pool) + "): ", 1, *p->pool, armies)) {
        return false;
    }

    Territory* target = toDefend[choice - 1];
    if (!p->queueOrder(new Deploy(p, target, armies))) {
        return false;
    }
    *p->pool -= armies;
    (*p->plannedArmies)[target] = p->getPlannedArmies(target) + armies;
    std::cout << std::endl << "Deploy order issued: " << armies << " armies to " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for an advance order towards one of the targets (the toAttack() list to
 * attack, the toDefend() list to defend), then for the neighboring territory the
 * armies leave from and how many of them move. Returns false if no order was issued.
 */
bool HumanPlayerStrategy::issueAdvanceOrder(const std::vector<Territory*>& targets, bool attack) {
    if (targets.empty()) {
        std::cout << std::endl << "There is no territory to " << (attack ? "attack." : "defend.") << std::endl;
        return false;
    }

    Territory* target = chooseTerritory(attack ? "Territory to attack" : "Territory to defend", targets);
    if (target == nullptr) {
        return false;
    }

    std::vector<Territory*> sources;
    for (Territory* territory : *p->territories) {
        if (territory != target && territory->isNeighborTo(target) && p->getPlannedArmies(territory) > 0) {
            sources.push_back(territory);
        }
    }
    if (sources.empty()) {
        std::cout << std::endl << "None of your territories next to " << target->getName()
                  << " has armies available." << std::endl;
        return false;
    }

    Territory* source = chooseTerritory("Territory to move armies from", sources);
    if (source == nullptr) {
        return false;
    }

    int available = p->getPlannedArmies(source);
    int armies = 0;
    if (!askNumber("Armies to move (1-" + std::to_string(available) + ", 0 to cancel): ",
                   0, available, armies) || armies == 0) {
        return false;
    }

    if (!p->queueOrder(new Advance(p, source, target, armies))) {
        return false;
    }
    (*p->plannedArmies)[source] = available - armies;
    std::cout << std::endl << "Advance order issued: " << armies << " armies from " << source->getName()
              << " to " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the target of a bomb card among the territories to attack.
 */
bool HumanPlayerStrategy::issueBombOrder(const std::vector<Territory*>& toAttack) {
    if (toAttack.empty()) {
        std::cout << std::endl << "There is no territory to bomb." << std::endl;
        return false;
    }
    Territory* target = chooseTerritory("Territory to bomb", toAttack);
    if (target == nullptr || !p->queueOrder(new Bomb(p, target))) {
        return false;
    }
    std::cout << std::endl << "Bomb order issued on " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the target of a blockade card among the territories to defend.
 */
bool HumanPlayerStrategy::issueBlockadeOrder(const std::vector<Territory*>& toDefend) {
    if (toDefend.empty()) {
        std::cout << std::endl << "You have no territory to blockade." << std::endl;
        return false;
    }
    Territory* target = chooseTerritory("Territory to blockade", toDefend);
    if (target == nullptr || !p->queueOrder(new Blockade(p, target))) {
        return false;
    }
    std::cout << std::endl << "Blockade order issued on " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the source, the target (any other territory of the player) and the
 * number of armies of an airlift card.
 */
bool HumanPlayerStrategy::issueAirliftOrder() {
    std::vector<Territory*> sources;
    for (Territory* territory : *p->territories) {
        if (p->getPlannedArmies(territory) > 0) {
            sources.push_back(territory);
        }
    }
    if (sources.empty()) {
        std::cout << std::endl << "None of your territories has armies available to airlift." << std::endl;
        return false;
    }

    Territory* source = chooseTerritory("Territory to airlift armies from", sources);
    if (source == nullptr) {
        return false;
    }

    std::vector<Territory*> targets;
    for (Territory* territory : *p->territories) {
        if (territory != source) {
            targets.push_back(territory);
        }
    }
    if (targets.empty()) {
        std::cout << std::endl << "You have no other territory to airlift to." << std::endl;
        return false;
    }

    Territory* target = chooseTerritory("Territory to airlift armies to", targets);
    if (target == nullptr) {
        return false;
    }

    int available = p->getPlannedArmies(source);
    int armies = 0;
    if (!askNumber("Armies to airlift (1-" + std::to_string(available) + ", 0 to cancel): ",
                   0, available, armies) || armies == 0) {
        return false;
    }

    if (!p->queueOrder(new Airlift(p, source, target, armies))) {
        return false;
    }
    (*p->plannedArmies)[source] = available - armies;
    std::cout << std::endl << "Airlift order issued: " << armies << " armies from " << source->getName()
              << " to " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the player to negotiate with (diplomacy card).
 */
bool HumanPlayerStrategy::issueNegotiateOrder(const std::vector<Player*>& players) {
    Player* other = choosePlayer(players);
    if (other == nullptr || !p->queueOrder(new Negotiate(p, other))) {
        return false;
    }
    std::cout << std::endl << "Negotiate order issued with " << other->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for a card in the hand and the order it creates. The card goes back to
 * the deck once its order is issued. A reinforcement card creates no order:
 * it puts armies in the pool right away. Returns false if nothing was played.
 */
bool HumanPlayerStrategy::issueCardOrder(const std::vector<Player*>& players, Deck* deck,
                                         const std::vector<Territory*>& toDefend,
                                         const std::vector<Territory*>& toAttack) {
    Hand* hand = p->hand;
    if (deck == nullptr || hand->isEmpty()) {
        std::cout << std::endl << "You have no card to play." << std::endl;
        return false;
    }

    std::cout << std::endl << "Your cards:" << std::endl;
    for (int i = 0; i < hand->size(); i++) {
        std::cout << "  " << (i + 1) << ". " << *hand->getCard(i) << std::endl;
    }
    int choice = 0;
    if (!askNumber("Card to play (1-" + std::to_string(hand->size()) + ", 0 to cancel): ",
                   0, hand->size(), choice) || choice == 0) {
        return false;
    }

    Card* card = hand->getCard(choice - 1);
    bool issued = false;
    switch (card->getType()) {
        case CardType::Bomb:
            issued = issueBombOrder(toAttack);
            break;
        case CardType::Blockade:
            issued = issueBlockadeOrder(toDefend);
            break;
        case CardType::Airlift:
            issued = issueAirliftOrder();
            break;
        case CardType::Diplomacy:
            issued = issueNegotiateOrder(players);
            break;
        case CardType::Reinforcement:
            *p->pool += REINFORCEMENT_CARD_ARMIES;
            std::cout << std::endl << "Reinforcement card played: " << REINFORCEMENT_CARD_ARMIES
                      << " armies added to your pool." << std::endl;
            issued = true;
            break;
    }

    if (issued) {
        hand->removeCard(card);
        if (!deck->addCard(card)) {
            delete card;
        }
    }
    return issued;
}

/**
 * Returns a copy of this strategy.
 */
PlayerStrategy* HumanPlayerStrategy::clone() const {
    return new HumanPlayerStrategy(*this);
}

/**
 * Returns the name of this kind of player.
 */
std::string HumanPlayerStrategy::getName() const {
    return "Human";
}

/**
 * Stream insertion operator.
 */
std::ostream& operator<<(std::ostream& out, const HumanPlayerStrategy& strategy) {
    out << static_cast<const PlayerStrategy&>(strategy);
    return out;
}

// ============================================================
// AggressivePlayerStrategy
// ============================================================

/**
 * Creates an aggressive strategy for the given player.
 */
AggressivePlayerStrategy::AggressivePlayerStrategy(Player* player) : PlayerStrategy(player) {}

/**
 * Copy constructor.
 */
AggressivePlayerStrategy::AggressivePlayerStrategy(const AggressivePlayerStrategy& other) : PlayerStrategy(other) {}

/**
 * Assignment operator.
 */
AggressivePlayerStrategy& AggressivePlayerStrategy::operator=(const AggressivePlayerStrategy& other) {
    PlayerStrategy::operator=(other);
    return *this;
}

/**
 * Destructor.
 */
AggressivePlayerStrategy::~AggressivePlayerStrategy() {}

/**
 * Skeleton: issues nothing and reports that there are no more orders.
 */
bool AggressivePlayerStrategy::issueOrder(const std::vector<Player*>& players, Deck* deck) {
    (void)players;
    (void)deck;
    return false;
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> AggressivePlayerStrategy::toAttack() const {
    return std::vector<Territory*>();
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> AggressivePlayerStrategy::toDefend() const {
    return std::vector<Territory*>();
}

/**
 * Returns a copy of this strategy.
 */
PlayerStrategy* AggressivePlayerStrategy::clone() const {
    return new AggressivePlayerStrategy(*this);
}

/**
 * Returns the name of this kind of player.
 */
std::string AggressivePlayerStrategy::getName() const {
    return "Aggressive";
}

/**
 * Stream insertion operator.
 */
std::ostream& operator<<(std::ostream& out, const AggressivePlayerStrategy& strategy) {
    out << static_cast<const PlayerStrategy&>(strategy);
    return out;
}

// ============================================================
// BenevolentPlayerStrategy
// ============================================================

/**
 * Creates a benevolent strategy for the given player.
 */
BenevolentPlayerStrategy::BenevolentPlayerStrategy(Player* player) : PlayerStrategy(player) {}

/**
 * Copy constructor.
 */
BenevolentPlayerStrategy::BenevolentPlayerStrategy(const BenevolentPlayerStrategy& other) : PlayerStrategy(other) {}

/**
 * Assignment operator.
 */
BenevolentPlayerStrategy& BenevolentPlayerStrategy::operator=(const BenevolentPlayerStrategy& other) {
    PlayerStrategy::operator=(other);
    return *this;
}

/**
 * Destructor.
 */
BenevolentPlayerStrategy::~BenevolentPlayerStrategy() {}

/**
 * Skeleton: issues nothing and reports that there are no more orders.
 */
bool BenevolentPlayerStrategy::issueOrder(const std::vector<Player*>& players, Deck* deck) {
    (void)players;
    (void)deck;
    return false;
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> BenevolentPlayerStrategy::toAttack() const {
    return std::vector<Territory*>();
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> BenevolentPlayerStrategy::toDefend() const {
    return std::vector<Territory*>();
}

/**
 * Returns a copy of this strategy.
 */
PlayerStrategy* BenevolentPlayerStrategy::clone() const {
    return new BenevolentPlayerStrategy(*this);
}

/**
 * Returns the name of this kind of player.
 */
std::string BenevolentPlayerStrategy::getName() const {
    return "Benevolent";
}

/**
 * Stream insertion operator.
 */
std::ostream& operator<<(std::ostream& out, const BenevolentPlayerStrategy& strategy) {
    out << static_cast<const PlayerStrategy&>(strategy);
    return out;
}

// ============================================================
// NeutralPlayerStrategy
// ============================================================

/**
 * Creates a neutral strategy for the given player.
 */
NeutralPlayerStrategy::NeutralPlayerStrategy(Player* player) : PlayerStrategy(player) {}

/**
 * Copy constructor.
 */
NeutralPlayerStrategy::NeutralPlayerStrategy(const NeutralPlayerStrategy& other) : PlayerStrategy(other) {}

/**
 * Assignment operator.
 */
NeutralPlayerStrategy& NeutralPlayerStrategy::operator=(const NeutralPlayerStrategy& other) {
    PlayerStrategy::operator=(other);
    return *this;
}

/**
 * Destructor.
 */
NeutralPlayerStrategy::~NeutralPlayerStrategy() {}

/**
 * Skeleton: issues nothing and reports that there are no more orders.
 */
bool NeutralPlayerStrategy::issueOrder(const std::vector<Player*>& players, Deck* deck) {
    (void)players;
    (void)deck;
    return false;
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> NeutralPlayerStrategy::toAttack() const {
    return std::vector<Territory*>();
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> NeutralPlayerStrategy::toDefend() const {
    return std::vector<Territory*>();
}

/**
 * Returns a copy of this strategy.
 */
PlayerStrategy* NeutralPlayerStrategy::clone() const {
    return new NeutralPlayerStrategy(*this);
}

/**
 * Returns the name of this kind of player.
 */
std::string NeutralPlayerStrategy::getName() const {
    return "Neutral";
}

/**
 * Stream insertion operator.
 */
std::ostream& operator<<(std::ostream& out, const NeutralPlayerStrategy& strategy) {
    out << static_cast<const PlayerStrategy&>(strategy);
    return out;
}

// ============================================================
// CheaterPlayerStrategy
// ============================================================

/**
 * Creates a cheater strategy for the given player.
 */
CheaterPlayerStrategy::CheaterPlayerStrategy(Player* player) : PlayerStrategy(player) {}

/**
 * Copy constructor.
 */
CheaterPlayerStrategy::CheaterPlayerStrategy(const CheaterPlayerStrategy& other) : PlayerStrategy(other) {}

/**
 * Assignment operator.
 */
CheaterPlayerStrategy& CheaterPlayerStrategy::operator=(const CheaterPlayerStrategy& other) {
    PlayerStrategy::operator=(other);
    return *this;
}

/**
 * Destructor.
 */
CheaterPlayerStrategy::~CheaterPlayerStrategy() {}

/**
 * Skeleton: issues nothing and reports that there are no more orders.
 */
bool CheaterPlayerStrategy::issueOrder(const std::vector<Player*>& players, Deck* deck) {
    (void)players;
    (void)deck;
    return false;
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> CheaterPlayerStrategy::toAttack() const {
    return std::vector<Territory*>();
}

/**
 * Skeleton: returns no territory.
 */
std::vector<Territory*> CheaterPlayerStrategy::toDefend() const {
    return std::vector<Territory*>();
}

/**
 * Returns a copy of this strategy.
 */
PlayerStrategy* CheaterPlayerStrategy::clone() const {
    return new CheaterPlayerStrategy(*this);
}

/**
 * Returns the name of this kind of player.
 */
std::string CheaterPlayerStrategy::getName() const {
    return "Cheater";
}

/**
 * Stream insertion operator.
 */
std::ostream& operator<<(std::ostream& out, const CheaterPlayerStrategy& strategy) {
    out << static_cast<const PlayerStrategy&>(strategy);
    return out;
}