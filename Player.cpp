// Player.cpp
// COMP 345 - Assignment 2: Player
// Implementation of the Player class declared in Player.h.

#include "Player.h"

#include "PlayerStrategies.h"

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
    negotiatedWith(new std::vector<Player*>()),
    strategy(new HumanPlayerStrategy(this)){}


/**
 * Copy constructor: deep copy of the name, hand, orders, pool and strategy.
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
    negotiatedWith(new std::vector<Player*>(*other.negotiatedWith)),
    strategy(other.strategy->clone()){
    strategy->setPlayer(this);
}

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
        delete strategy;
        strategy = other.strategy->clone();
        strategy->setPlayer(this);
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
    delete strategy;
}

/**
 * Replaces the player's strategy (the old one is deleted). The player takes
 * ownership of the new strategy. A null strategy is ignored.
 */
void Player::setStrategy(PlayerStrategy* newStrategy){
    if (newStrategy == nullptr || newStrategy == strategy) {
        return;
    }
    delete strategy;
    strategy = newStrategy;
    strategy->setPlayer(this);
}

/**
 * Returns the strategy currently used by the player.
 */
PlayerStrategy* Player::getStrategy() const{
    return strategy;
}

/**
 * Strategy pattern: delegates to the strategy of the player.
 */
std::vector<Territory*> Player::toDefend() const{
    return strategy->toDefend();
}

/**
 * Strategy pattern: delegates to the strategy of the player.
 */
std::vector<Territory*> Player::toAttack() const{
    return strategy->toAttack();
}

/**
 * Strategy pattern: delegates to the strategy of the player.
 */
bool Player::issueOrder(const std::vector<Player*>& players, Deck* deck){
    return strategy->issueOrder(players, deck);
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
 * strategy, pool, hand of cards and list of orders.
 */
std::ostream& operator<<(std::ostream& out, const Player& player){
    out << "Player " << player.getName() << "\n Territories:";
    for (Territory* t : player.getTerritories()){
        out << " [" << *t << "]";
    }
    out << "\n Strategy: " << *player.getStrategy();
    out << "\n Pool: " << player.getPool();
    out << "\n Hand: " << *player.getHand();
    out << "\n Orders: " << *player.getOrders();
    return out;
}