// Player.cpp
// COMP 345 - Assignment 2: Player
// Implementation of the Player class declared in Player.h.

#include "Player.h"

/**
 * Creates a player with the given name, no territories, an empty hand,
 * an empty list of orders and an empty pool.
 */
Player::Player(const std::string& name)
    : name(new std::string(name)),
    territories(new std::vector<Territory*>()),
    attackTargets(new std::vector<Territory*>()),
    hand(new Hand()),
    orders(new OrdersList()),
    pool(new int(0)),
    negotiatedWith(new std::vector<Player*>()){}


/**
 * Copy constructor: deep copy of the name, hand, orders and pool.
 * The territory lists hold the same Territory pointers, since the Map owns them.
 */
Player::Player(const Player& other)
    : name(new std::string(*other.name)),
    territories(new std::vector<Territory*>(*other.territories)),
    attackTargets(new std::vector<Territory*>(*other.attackTargets)),
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
        *attackTargets = *other.attackTargets;
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
    delete attackTargets;
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
 * Adds a territory to the ones returned by toAttack().
 */
void Player::addAttackTarget(Territory* territory){
    attackTargets->push_back(territory);
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
 * Returns the territories to be defended. For now this is an arbitrary
 * list: the first half (rounded up) of the territories the player owns.
 */
std::vector<Territory*>Player::toDefend() const{
    std::vector<Territory*> result;
    size_t count = (territories->size() + 1) / 2;
    for (size_t i = 0; i < count; i++){
        result.push_back((*territories)[i]);
    }
    return result;
}

/**
 * Returns the territories to be attacked. For now this is an arbitrary
 * list: the targets added with addAttackTarget().
 */
std::vector<Territory*> Player::toAttack() const {
    return *attackTargets;
}

/**
 * Creates an order of the given type ("deploy" or "advance") and adds it to
 * the player's list of orders. Returns false for any other type, or if the
 * list of orders is full (the order is then deleted so it does not leak).
 */
bool Player::issueOrder(const std::string& orderType){
    Order* order = nullptr;
    if (orderType == "deploy") {
        order = new Deploy();
    } else if (orderType == "advance") {
        order = new Advance();
    } else {
        std::cout << "Invalid order type: " << orderType << std::endl;
        return false;
    }
    if (!orders->add(order)) {
        delete order;
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