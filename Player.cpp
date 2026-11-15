// Player.cpp
// COMP 345 - Assignment 2: Player
// Implementation of the Player class declared in Player.h.

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

/**
 * Creates a player with the given name, no territories, an empty hand,
 * an empty list of orders and an empty pool.
 */
Player::Player(const std::string& name)
    : name(new std::string(name)),
    territories(new std::vector<Territory*>()),
    defendTargets(new std::vector<Territory*>()),
    attackTargets(new std::vector<Territory*>()),
    targetsChosen(new bool(false)),
    plannedArmies(new std::map<Territory*, int>()),
    hand(new Hand()),
    orders(new OrdersList()),
    pool(new int(0)),
    negotiatedWith(new std::vector<Player*>()){}


/**
 * Copy constructor: deep copy of the name, hand, orders and pool.
 * The territory lists hold the same pointers, since the Map owns the territories.
 */
Player::Player(const Player& other)
    : name(new std::string(*other.name)),
    territories(new std::vector<Territory*>(*other.territories)),
    defendTargets(new std::vector<Territory*>(*other.defendTargets)),
    attackTargets(new std::vector<Territory*>(*other.attackTargets)),
    targetsChosen(new bool(*other.targetsChosen)),
    plannedArmies(new std::map<Territory*, int>(*other.plannedArmies)),
    hand(new Hand(*other.hand)),
    orders(new OrdersList(*other.orders)),
    pool(new int(*other.pool)),
    negotiatedWith(new std::vector<Player*>(*other.negotiatedWith)){}

/**
 * Assignment operator: same copying rules as the copy constructor,
 * with a self-assignment check.
 */
Player& Player::operator=(const Player& other){
    if (this != &other) {
        *name = *other.name;
        *territories = *other.territories;
        *defendTargets = *other.defendTargets;
        *attackTargets = *other.attackTargets;
        *targetsChosen = *other.targetsChosen;
        *plannedArmies = *other.plannedArmies;
        *hand = *other.hand;
        *orders = *other.orders;
        *pool = *other.pool;
        *negotiatedWith = *other.negotiatedWith;
    }
    return *this;
}

/**
 * Destructor: frees everything the player owns. The territories themselves
 * are not deleted, because the Map owns them.
 */
Player::~Player(){
    delete name;
    delete territories;
    delete defendTargets;
    delete attackTargets;
    delete targetsChosen;
    delete plannedArmies;
    delete hand;
    delete orders;
    delete pool;
    delete negotiatedWith;
}

/**
 * Returns the player's name.
 */
std::string Player::getName() const{
    return *name;
}

/**
 * Adds a territory to the ones the player owns.
 */
void Player::addTerritory(Territory* territory){
    territories->push_back(territory);
}

/**
 * Forgets all the territories of the player.
 */
void Player::clearTerritories(){
    territories->clear();
}

/**
 * Returns the territories the player owns.
 */
std::vector<Territory*> Player::getTerritories() const{
    return *territories;
}

/**
 * Returns the player's hand of cards.
 */
Hand* Player::getHand() const{
    return hand;
}

/**
 * Returns the player's list of orders.
 */
OrdersList* Player::getOrders() const{
    return orders;
}

/**
 * Returns the number of armies in the player's pool.
 */
int Player::getPool() const{
    return *pool;
}

/**
 * Sets the number of armies in the player's pool.
 */
void Player::setPool(int armies){
    *pool = armies;
}

/**
 * Prints all the territories the player owns, asks (through the console) which
 * ones to defend and returns them. The answer is a list of numbers separated by
 * spaces, 0 or an empty line means none.
 */
std::vector<Territory*> Player::toDefend() const{
    std::vector<Territory*> result;
    printTerritories("Your territories", *territories);

    std::vector<int> selection;
    if (territories->empty()
        || !askSelection("Territories to defend (numbers separated by spaces, 0 for none): ",
                         static_cast<int>(territories->size()), selection)) {
        return result;
    }
    for (int number : selection) {
        result.push_back((*territories)[number - 1]);
    }
    return result;
}

/**
 * Prints all the territories next to one of the player's territories and owned
 * by somebody else, asks (through the console) which ones to attack and returns
 * them. The answer is a list of numbers separated by spaces, 0 or an empty line
 * means none.
 */
std::vector<Territory*> Player::toAttack() const{
    std::vector<Territory*> neighbors;
    for (Territory* territory : *territories) {
        for (Territory* neighbor : territory->getNeighbors()) {
            if (neighbor->getOwner() != this
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
 * Asks toDefend() and toAttack() and remembers the answers for the rest of the turn.
 */
void Player::chooseTargets(){
    *defendTargets = toDefend();
    *attackTargets = toAttack();
    *targetsChosen = true;
}

/**
 * Starts a new issuing phase: every territory of the player is planned with the
 * armies it has now. Deploy, advance and airlift orders issued afterwards
 * change this plan, so the player cannot commit the same armies twice.
 * The territories chosen to defend and attack are forgotten.
 */
void Player::resetPlannedArmies(){
    plannedArmies->clear();
    for (Territory* territory : *territories) {
        (*plannedArmies)[territory] = territory->getArmies();
    }
    defendTargets->clear();
    attackTargets->clear();
    *targetsChosen = false;
}

/**
 * Returns the armies that can still be used from a territory in this issuing phase.
 */
int Player::getPlannedArmies(Territory* territory) const{
    auto found = plannedArmies->find(territory);
    return found == plannedArmies->end() ? territory->getArmies() : found->second;
}

/**
 * Adds an order to the list of orders. If the list is full the order is
 * deleted so it does not leak, and false is returned.
 */
bool Player::queueOrder(Order* order){
    if (!orders->add(order)) {
        delete order;
        std::cout << std::endl << "Your list of orders is full." << std::endl;
        return false;
    }
    return true;
}

/**
 * Prints a numbered list of territories with their owner and armies.
 */
void Player::printTerritories(const std::string& title, const std::vector<Territory*>& list) const{
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
        if (territory->getOwner() == this) {
            std::cout << " (available for orders: " << getPlannedArmies(territory) << ")";
        }
        std::cout << std::endl;
    }
}

/**
 * Records that this player has agreed not to attack 'other' this turn.
 * Ignores null pointers, self-negotiation and duplicates.
 */
void Player::addNegotiation(Player* other){
    if (other == nullptr || other == this){
        return;
    }
    if (!isNegotiatingWith(other)){
        negotiatedWith->push_back(other);
    }
}

/**
 * Prints a list of territories and asks for one of them.
 * Returns nullptr if the player cancels or the input ends.
 */
Territory* Player::chooseTerritory(const std::string& title, const std::vector<Territory*>& list) const{
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
Player* Player::choosePlayer(const std::vector<Player*>& players) const{
    std::vector<Player*> others;
    for (Player* candidate : players) {
        if (candidate != this) {
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
bool Player::issueDeployOrder(const std::vector<Territory*>& toDefend){
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
    if (!askNumber("Armies to deploy (1-" + std::to_string(*pool) + "): ", 1, *pool, armies)) {
        return false;
    }

    Territory* target = toDefend[choice - 1];
    if (!queueOrder(new Deploy(this, target, armies))) {
        return false;
    }
    *pool -= armies;
    (*plannedArmies)[target] = getPlannedArmies(target) + armies;
    std::cout << std::endl << "Deploy order issued: " << armies << " armies to " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for an advance order towards one of the targets (the toAttack() list to
 * attack, the toDefend() list to defend), then for the neighboring territory the
 * armies leave from and how many of them move. Returns false if no order was issued.
 */
bool Player::issueAdvanceOrder(const std::vector<Territory*>& targets, bool attack){
    if (targets.empty()) {
        std::cout << std::endl << "There is no territory to " << (attack ? "attack." : "defend.") << std::endl;
        return false;
    }

    Territory* target = chooseTerritory(attack ? "Territory to attack" : "Territory to defend", targets);
    if (target == nullptr) {
        return false;
    }

    std::vector<Territory*> sources;
    for (Territory* territory : *territories) {
        if (territory != target && territory->isNeighborTo(target) && getPlannedArmies(territory) > 0) {
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

    int available = getPlannedArmies(source);
    int armies = 0;
    if (!askNumber("Armies to move (1-" + std::to_string(available) + ", 0 to cancel): ",
                   0, available, armies) || armies == 0) {
        return false;
    }

    if (!queueOrder(new Advance(this, source, target, armies))) {
        return false;
    }
    (*plannedArmies)[source] = available - armies;
    std::cout << std::endl << "Advance order issued: " << armies << " armies from " << source->getName()
              << " to " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the target of a bomb card among the territories to attack.
 */
bool Player::issueBombOrder(const std::vector<Territory*>& toAttack){
    if (toAttack.empty()) {
        std::cout << std::endl << "There is no territory to bomb." << std::endl;
        return false;
    }
    Territory* target = chooseTerritory("Territory to bomb", toAttack);
    if (target == nullptr || !queueOrder(new Bomb(this, target))) {
        return false;
    }
    std::cout << std::endl << "Bomb order issued on " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the target of a blockade card among the territories to defend.
 */
bool Player::issueBlockadeOrder(const std::vector<Territory*>& toDefend){
    if (toDefend.empty()) {
        std::cout << std::endl << "You have no territory to blockade." << std::endl;
        return false;
    }
    Territory* target = chooseTerritory("Territory to blockade", toDefend);
    if (target == nullptr || !queueOrder(new Blockade(this, target))) {
        return false;
    }
    std::cout << std::endl << "Blockade order issued on " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the source, the target (any other territory of the player) and the
 * number of armies of an airlift card.
 */
bool Player::issueAirliftOrder(){
    std::vector<Territory*> sources;
    for (Territory* territory : *territories) {
        if (getPlannedArmies(territory) > 0) {
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
    for (Territory* territory : *territories) {
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

    int available = getPlannedArmies(source);
    int armies = 0;
    if (!askNumber("Armies to airlift (1-" + std::to_string(available) + ", 0 to cancel): ",
                   0, available, armies) || armies == 0) {
        return false;
    }

    if (!queueOrder(new Airlift(this, source, target, armies))) {
        return false;
    }
    (*plannedArmies)[source] = available - armies;
    std::cout << std::endl << "Airlift order issued: " << armies << " armies from " << source->getName()
              << " to " << target->getName() << "." << std::endl;
    return true;
}

/**
 * Asks for the player to negotiate with (diplomacy card).
 */
bool Player::issueNegotiateOrder(const std::vector<Player*>& players){
    Player* other = choosePlayer(players);
    if (other == nullptr || !queueOrder(new Negotiate(this, other))) {
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
bool Player::issueCardOrder(const std::vector<Player*>& players, Deck* deck,
                            const std::vector<Territory*>& toDefend,
                            const std::vector<Territory*>& toAttack){
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
            *pool += REINFORCEMENT_CARD_ARMIES;
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
 * Asks the player what order to issue. While the pool still has armies only a
 * deploy order is possible. Once the pool is empty the player can advance
 * armies to defend (targets chosen with toDefend()), advance armies to attack
 * (targets chosen with toAttack()), play a card, choose the targets again, or
 * end the turn. toDefend() and toAttack() are asked once per turn, the first
 * time the player is asked for an order. Orders are not validated
 * here, only when they are executed.
 * Returns true if an order was issued, false if the player has no more orders this turn.
 */
bool Player::issueOrder(const std::vector<Player*>& players, Deck* deck){
    std::cout << std::endl << "Armies left to deploy: " << *pool << std::endl
              << "Orders issued so far: " << orders->size() << std::endl;

    if (!*targetsChosen) {
        chooseTargets();
    }

    if (*pool > 0) {
        std::cout << std::endl << "You still have armies to deploy, so only a deploy order is allowed." << std::endl;
        // deploying needs at least one territory to defend
        while (defendTargets->empty()) {
            if (std::cin.eof()) {
                return false;
            }
            std::cout << std::endl << "You must choose at least one territory to defend to deploy your armies." << std::endl;
            *defendTargets = toDefend();
        }
        return issueDeployOrder(*defendTargets);
    }

    while (true) {
        if (orders->size() >= orders->capacity()) {
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
            issued = issueAdvanceOrder(*defendTargets, false);
        } else if (choice == 2) {
            issued = issueAdvanceOrder(*attackTargets, true);
        } else if (choice == 3) {
            issued = issueCardOrder(players, deck, *defendTargets, *attackTargets);
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
 * Tells whether is this player is currently negotiating with 'other'.
 */
bool Player::isNegotiatingWith(Player* other) const{
   for (Player* p : *negotiatedWith){
        if (p == other)
            return true;
   }
   return false;
}

void Player::clearNegotiations(){
    negotiatedWith->clear();
}

/**
 * Stream insertion operator: prints the player's name, territories,
 * pool, hand of cards and list of orders.
 */
std::ostream& operator<<(std::ostream& out, const Player& player){
    out << "Player " << player.getName() << "\n Territories:";
    for (Territory* t : player.getTerritories()){
        out << " [" << *t << "]";
    }
    out << "\n Pool: " << player.getPool();
    out << "\n Hand: " << *player.getHand();
    out << "\n Orders: " << *player.getOrders();
    return out;
}