#include "Player.h"

Player::Player(const std::string& name)
    : name(new std::string(name)),
    territories(new std::vector<Territory*>()),
    attackTargets(new std::vector<Territory*>()),
    hand(new Hand()),
    orders(new OrdersList()){}

Player::Player(const Player& other)
    : name(new std::string(*other.name)),
    territories(new std::vector<Territory*>(*other.territories)),
    attackTargets(new std::vector<Territory*>(*other.attackTargets)),
    hand(new Hand(*other.hand)),
    orders(new OrdersList(*other.orders)){}

Player& Player::operator=(const Player& other){
    if (this != &other) {
        *name = *other.name;
        *territories = *other.territories;
        *attackTargets = *other.attackTargets;
        *hand = *other.hand;
        *orders = *other.orders;
    }
    return *this;
}

Player::~Player(){
    delete name;
    delete territories;
    delete attackTargets;
    delete hand;
    delete orders;
}

std::string Player::getName() const{
    return *name;
}

void Player::addTerritory(Territory* territory){
    territories->push_back(territory);
}

void Player::addAttackTarget(Territory* territory){
    attackTargets->push_back(territory);
}

std::vector<Territory*> Player::getTerritories() const{
    return *territories;
}

Hand* Player::getHand() const{
    return hand;
}

OrdersList* Player::getOrders() const{
    return orders;
}

std::vector<Territory*>Player::toDefend() const{
    std:vector<Territory*> result;
    size_t count = (territories->size() + 1) / 2;
    for (size_t i = 0; i < count; i++){
        result.push_back((*territories)[i]);
    }
    return result;
}

std::vector<Territory*> Player::toAttack() const {
    return *attackTargets;
}

bool Player::issueOrder(const std::string& orderType){
    if (orderType == "deploy") {
        orders->add(new Deploy());
        return true;
    }
    if (orderType == "advance") {
        orders->add(new Advance());
        return true;
    }
    std::cout <<"Invalid order type: " << orderType << std::end1;
    return false;    
}

std::ostream& operator<<(std::ostream& out, const Player& player){
    out << "Player " << player.getName() << "\n Territories:";
    for (Territory* t : player.getTerritories()){
        out << " [" << *t << "]";
    }
    out << "\n Hand: " << *player.getHand();
    out << "\n Orders: " << *player.getOrders();
    return out;
}