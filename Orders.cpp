// Orders.cpp
// COMP 345 - Assignment 1 - Part 3
// Order actions are NOT implemented at this point.

#include "Orders.h"
#include "Map.h"
#include "Player.h"

#include <iostream>
#include <sstream>

// ============================================================
// Order (base) implementation
// ============================================================

// Default constructor: allocate an empty effect string on the heap.
Order::Order() : effect(new std::string()) {}

// Copy constructor: deep copy the effect string.
Order::Order(const Order& other) : effect(new std::string(*other.effect)) {}

/// @brief Assignment operator for Order: deep copy with self-assignment check.
/// @param other The Order object.
/// @return The current Order object.
Order& Order::operator=(const Order& other) {
    if (this != &other)
        *effect = *other.effect;
    return *this;
}

// Destructor: free the owned effect string.
Order::~Order() { delete effect; }

/// @brief Setter for effect 
/// @param e The string to set the effect to.
void Order::setEffect(const std::string& e) { *effect = e; }

/// @brief Getter for effect
/// @return The string for the effect
std::string Order::getEffect() const { return *effect; }

/// @brief Stream inseration operator: Prints order description and the effect if it executed.
/// @param out The output stream object.
/// @param o The order object.
/// @return The output of the order/effect description.
std::ostream& operator<<(std::ostream& out, const Order& o) {
    out << o.getDescription();
    if (!o.getEffect().empty())
        out << " [effect: " << o.getEffect() << "]";
    return out;
}

// ============================================================
// Deploy implementation
//
// Brief Description:
// Place some armies on one of the current player’s territories.       
// ============================================================

// Default constructor: intializes pointers to nullptr and allocates a new int for armies at 0.
Deploy::Deploy()
    : Order(), player(nullptr), target(nullptr), armies(new int(0)) {}

// Parameterized constructor: initializes all member and as well allocates a new int for armies via a provided value.
Deploy::Deploy(Player* player, Territory* target, int armies)
    : Order(), player(player), target(target), armies(new int(armies)) {}

// Copy constructor: deep copies the armies pointer.
Deploy::Deploy(const Deploy& other)
    : Order(other), player(other.player), target(other.target),
      armies(new int(*other.armies)) {}

/// @brief Assignment operator for Deploy: calls base class assignment then copies own members.
/// @param other The Deploy object.
/// @return The current Deploy object.
Deploy& Deploy::operator=(const Deploy& other) {
    if (this != &other) {
        Order::operator=(other); // base class handle its own members
        player  = other.player;
        target  = other.target;
        *armies = *other.armies; // deep copy the armies
    }
    return *this;
}

/// Destructor: frees the dynamically allocated armies.
Deploy::~Deploy() { delete armies; }

/// @brief Validates if the current army is valid to execute.
/// @return True if valid, false otherwise.
bool Deploy::validate() const {
    // Base check
    if (!armies || *armies <= 0) return false;
    
    if (!player || !target)       return false;
    if (target->getOwner() != player) return false;

    return true;
}

/// @brief Executes the deploy and sets its effect for display.
void Deploy::execute() {
    if (!validate()) {
        setEffect("invalid order (not executed)");
        return;
    }

    target->setArmies(target->getArmies() + *armies);
    std::ostringstream oss;
    oss << "PLACEHOLDER - would deploy " << *armies
        << " armies (no game state changed)";
    setEffect(oss.str());
}

/// @brief Deep copy helper that is used by OrdersList.
Order* Deploy::newCopy() const { return new Deploy(*this); }

/// @brief Displays the description of the deploy order.
/// @return The string that displays the description of the order.
std::string Deploy::getDescription() const {
    std::ostringstream oss;
    oss << "Deploy " << *armies << " armies";
    return oss.str();
}

// ============================================================
// Advance implementation
//
// Brief description:
// Move some armies from one of the current player’s territories (source) to an adjacent territory 
// (target). If the target territory belongs to the current player, the armies are moved to the target 
// territory. If the target territory belongs to another player, an attack happens between the two 
// territories.
// ============================================================

// Default constructor: intializes pointers to nullptr and alocates a new int for armies at 0.
Advance::Advance()
    : Order(), player(nullptr), from(nullptr), to(nullptr), armies(new int(0)) {}

// Parameterized constructor: initalizes members with provided arguments and allocates a new int for armies with provided value.
Advance::Advance(Player* player, Territory* from, Territory* to, int armies)
    : Order(), player(player), from(from), to(to), armies(new int(armies)) {}

// Copy constructor: calls base class copy constructor, copies the pointer values, and deep copies armies by allocating a new int.
Advance::Advance(const Advance& other)
    : Order(other), player(other.player), from(other.from), to(other.to),
      armies(new int(*other.armies)) {}

/// @brief Assignment operator for Advance: checks for self assignment then calls base class assignment operator. Copies pointers and deep copies armies.
/// @param other The Advance object.
/// @return The current Advance object.
Advance& Advance::operator=(const Advance& other) {
    if (this != &other) {
        Order::operator=(other);
        player  = other.player;
        from    = other.from;
        to      = other.to;
        *armies = *other.armies;
    }
    return *this;
}

// Destructor: frees the dynamically allocated armies.
Advance::~Advance() { delete armies; }

/// @brief Validates the Advance order -- checks if the army is a valid size or follows suit with the rules.
/// @return True if valid, false otherwise
bool Advance::validate() const {
    if (!armies || *armies <= 0) return false;
    if (!player || !from || !to)       return false;
    if (from->getOwner() != player)     return false; 
    if (!from->isNeighborTo(to))         return false;
    if (*armies > from->getArmies())   return false;
    return true;
}

/// @brief Executes the Advance order -- Checks if the Territory exists and will simulate the battle.
void Advance::execute() {
    if (!validate()) {
        setEffect("invalid order (not executed)");
        return;
    }
    std::ostringstream oss;

    if(from->getOwner() == player){
        from->setArmies(from->getArmies() - *armies);
        to->setArmies(to->getArmies() + *armies);
        oss << "moved " << *armies << " armies between own territories.";
    } else {
        int atk = *armies, def = to->getArmies();
        int atkKills = 0, defKills = 0;
        for (int i = 0; i < atk; ++i) if((std::rand() % 100) < 60) ++atkKills;
        for (int i = 0; i < def; ++i) if((std::rand() % 100) < 70) ++defKills;

        int atkLeft = atk - defKills;   if (atkLeft < 0) atkLeft = 0;
        int defLeft = def - atkKills;   if (defLeft < 0) defLeft = 0;

        from->setArmies(from->getArmies() - (atk - atkLeft));

        if (defLeft == 0){
            to->setOwner(player);
            to->setArmies(atkLeft);
            oss << "attack succeeded! Capture with: " << atkLeft << " armies.";
        } else {
            to->setArmies(defLeft);
            oss << " attacked failed! Defender has: " << defLeft << " armies left.";
        }
    }
    setEffect(oss.str());
}

/// @brief Deep copy helper that is used by OrdersList.
Order* Advance::newCopy() const { return new Advance(*this); }

/// @brief Advance order description.
/// @return The string that contains the Advance order description.
std::string Advance::getDescription() const {
    std::ostringstream oss;
    oss << "Advance " << *armies << " armies";
    return oss.str();
}

// ============================================================
// Bomb implementation
//
// Brief Description:
// Destroy half of the armies located on an opponent’s territory that is adjacent to one of the current 
// player’s territories.
// ============================================================

// Default constructor: initalizes pointers to nullptr.
Bomb::Bomb() : Order(), player(nullptr), target(nullptr) {}

// Parameterized constructor: intializes members with provided arguments.
Bomb::Bomb(Player* player, Territory* target)
    : Order(), player(player), target(target) {}

// Copy constructor: calls base class copy constructor and copies the pointer values.
Bomb::Bomb(const Bomb& other)
    : Order(other), player(other.player), target(other.target) {}

/// @brief Assignment operator for Bomb: checks self assignment, calls base class assignment then copies pointers.
/// @param other Bomb object.
/// @return The current Bomb object.
Bomb& Bomb::operator=(const Bomb& other) {
    if (this != &other) {
        Order::operator=(other);
        player = other.player;
        target = other.target;
    }
    return *this;
}

// Destructor: nothing to free, no owned pointers.
Bomb::~Bomb() {}

/// @brief Validates if Bomb order is valid: checks if target is valid.
/// @return True if valid, false otherwise.
bool Bomb::validate() const {  
    if (!player || !target)               return false;
    if (target->getOwner() == player)     return false; 
    bool ownsAdjacent = false;
    for(Territory* t : player->getTerritories()){
        if(t->isNeighborTo(target)){
            ownsAdjacent = true;
        }
    }
    if(!ownsAdjacent) 
        return false;
    else
        return true;
}

/// @brief Executes Bomb order and sets its effect.
void Bomb::execute() {
    if (!validate()) {
        setEffect("invalid order (not executed)");
        return;
    }
    target->setArmies(target->getArmies() / 2);
    setEffect("PLACEHOLDER - would bomb target (no game state changed)");
}

/// @brief Deep copy helper that is used by OrdersList.
Order* Bomb::newCopy() const { return new Bomb(*this); }

/// @brief Description of the Bomb order.
/// @return String that represents the Bomb order.
std::string Bomb::getDescription() const { return "Bomb"; }

// ============================================================
// Blockade implementation
//
// Brief Description:
// Triple the number of armies on one of the current player’s territories and make it a neutral territory. 
// ============================================================

// Default constructor: initializes pointers to nullptr.
Blockade::Blockade() : Order(), player(nullptr), target(nullptr) {}

// Parameterized constructor: initializes members with provided arguments.
Blockade::Blockade(Player* player, Territory* target)
    : Order(), player(player), target(target) {}

// Copy constructor: calls base class copy constructor and copies the pointer values.
Blockade::Blockade(const Blockade& other)
    : Order(other), player(other.player), target(other.target) {}

/// @brief Assignment operator for Blockade: checks for self-assigment, calls base class assignment then copies pointers.
/// @param other Blockade object.
/// @return THe current Blockade object.
Blockade& Blockade::operator=(const Blockade& other) {
    if (this != &other) {
        Order::operator=(other);
        player = other.player;
        target = other.target;
    }
    return *this;
}

// Destructor: nothing to free, no owned pointers.
Blockade::~Blockade() {}

/// @brief Validates if a Blockade is valid: if such player or target is valid.
/// @return True if valid, false otherwise.
bool Blockade::validate() const {
       if (!player || !target)       return false;
       if (target->getOwner() != player)     return false; 
    return true;
}

/// @brief Executes the Blockade order and sets its effect.
void Blockade::execute() {
    if (!validate()) {
        setEffect("invalid order (not executed)");
        return;
    }

    target->setArmies(target->getArmies() * 3);
    target->setOwner(nullptr);
    setEffect("PLACEHOLDER - would blockade target (no game state changed)");
}

/// @brief Deep copy helper that is used by OrdersList.
Order* Blockade::newCopy() const { return new Blockade(*this); }

/// @brief Grabs the description of Blockade order.
/// @return The string that represents the Blockade order.
std::string Blockade::getDescription() const { return "Blockade"; }

// ============================================================
// Airlift implementation
//
// Brief Description:
// Advance some armies from one of the current player’s territories to any another territory. 
// ============================================================

// Default constructor: initializes pointers to nullptr and allocates a new int for armies at 0.
Airlift::Airlift()
    : Order(), player(nullptr), from(nullptr), to(nullptr), armies(new int(0)) {}

// Parameterized constructor: initializes members with provided arguments and allocates a new int for armies with provided value.
Airlift::Airlift(Player* player, Territory* from, Territory* to, int armies)
    : Order(), player(player), from(from), to(to), armies(new int(armies)) {}

// Copy constructor: calls base class copy constructor, copies pointers and deep copies armies by allocating a new int.
Airlift::Airlift(const Airlift& other)
    : Order(other), player(other.player), from(other.from), to(other.to),
      armies(new int(*other.armies)) {}

/// @brief Assignment operator for Airlift: checks self assignment, calls base class assignment, copies pointers then deep copies the armies.
/// @param other Airlift object.
/// @return The current Airlift object.
Airlift& Airlift::operator=(const Airlift& other) {
    if (this != &other) {
        Order::operator=(other);
        player  = other.player;
        from    = other.from;
        to      = other.to;
        *armies = *other.armies;
    }
    return *this;
}

/// Destructor: frees the dynamically allocated armies.
Airlift::~Airlift() { delete armies; }

/// @brief Validates the Airlift order: checks if armies value is valid and if its territory location is valid.
/// @return True if valid, false otherwise.
bool Airlift::validate() const {
    if (!armies || *armies <= 0) return false;

    if (!player || !from || !to)       return false;
    if (from->getOwner() != player)    return false;
    if (*armies > from->getArmies())   return false;
    return true;
}

/// @brief Executes the Airlift order and sets its effect.
void Airlift::execute() {
    if (!validate()) {
        setEffect("invalid order (not executed)");
        return;
    }
  
    from->setArmies(from->getArmies() - *armies);
    to->setArmies(to->getArmies() + *armies);
    std::ostringstream oss;
    oss << "PLACEHOLDER - would airlift " << *armies
        << " armies (no game state changed)";
    setEffect(oss.str());
}

/// @brief Deep copy helper that is used by OrdersList.
Order* Airlift::newCopy() const { return new Airlift(*this); }

/// @brief Grabs the description of Airlift object.
/// @return The string that represents the Airlift order.
std::string Airlift::getDescription() const {
    std::ostringstream oss;
    oss << "Airlift " << *armies << " armies";
    return oss.str();
}

// ============================================================
// Negotiate
//
// Brief Description:
// Prevent attacks between the current player and another player until the end of the turn. 
// ============================================================

// Default constructor: initalizes pointers to nullptr.
Negotiate::Negotiate() : Order(), player(nullptr), otherPlayer(nullptr) {}

// Parameterized constructor: initializes members with provided arguments.
Negotiate::Negotiate(Player* player, Player* otherPlayer)
    : Order(), player(player), otherPlayer(otherPlayer) {}

// Copy constructor: calls base class copy constructor then copies the pointer values.
Negotiate::Negotiate(const Negotiate& other)
    : Order(other), player(other.player), otherPlayer(other.otherPlayer) {}

/// @brief Assignment operator for Negotiate: checks self-assigment, calls base class assignment then copies pointers.
/// @param other The Negotiate object.
/// @return The current Negotiate object.
Negotiate& Negotiate::operator=(const Negotiate& other) {
    if (this != &other) {
        Order::operator=(other);
        player      = other.player;
        otherPlayer = other.otherPlayer;
    }
    return *this;
}

// Destructor: nothing to free, no owned pointers.
Negotiate::~Negotiate() {}

/// @brief Validates the Negotiate order: checks if the target player is valid.
/// @return True if valid, false otherwise.
bool Negotiate::validate() const {
    // Basic check: you can't negotiate with yourself.
    if (player == otherPlayer) return false;
    if (!player || !otherPlayer) return false;
    return true;
}

/// @brief Executes the Negotiate order and sets its effect description.
void Negotiate::execute() {
    if (!validate()) {
        setEffect("invalid order (not executed)");
        return;
    }

    setEffect("PLACEHOLDER - would negotiate (no game state changed)");
}

/// @brief Deep copy helper that is used by OrdersList.
Order* Negotiate::newCopy() const { return new Negotiate(*this); }

/// @brief Grabs the description of the Negotiate object.
/// @return The string that represents the Negotiate order.
std::string Negotiate::getDescription() const { return "Negotiate"; }

// ============================================================
// OrdersList - Contains a list that holds all orders. 
// ============================================================

// Default constructor: allocates static array of Order* pointers, initializes count to 0, capacity to DEFAULT_CAPACITY and sets all array slots to nullptr.
OrdersList::OrdersList()
    : orders(new Order*[DEFAULT_CAPACITY]),
      count(new int(0)),
      cap(new int(DEFAULT_CAPACITY))
{
    for (int i = 0; i < *cap; ++i) orders[i] = nullptr;
}

// Copy constructor: A deep copy that allocates a new array of the same capacity, copies count/cap and iterates through the other list call newCopy() on each Order.
OrdersList::OrdersList(const OrdersList& other)
    : orders(new Order*[*other.cap]),
      count(new int(*other.count)),
      cap(new int(*other.cap))
{
    for (int i = 0; i < *count; ++i) orders[i] = other.orders[i]->newCopy();
    for (int i = *count; i < *cap; ++i) orders[i] = nullptr;
}

/// @brief Assignment operator for OrderList: checks for self assignment, frees existing resources to prevent memory leaks then allocates new memory + deep copies
///        orders from other.
/// @param other The OrderList object.
/// @return The current OrderList object.
OrdersList& OrdersList::operator=(const OrdersList& other) {
    if (this != &other) {
        for (int i = 0; i < *count; ++i) delete orders[i];
        delete[] orders;

        *cap   = *other.cap;
        *count = *other.count;
        orders = new Order*[*cap];
        for (int i = 0; i < *count; ++i) orders[i] = other.orders[i]->newCopy();
        for (int i = *count; i < *cap; ++i) orders[i] = nullptr;
    }
    return *this;
}

// Destructor: iterates through the list and deletes each Order object then deletes the array of pointers, count and capacity pointers to prevent memory leak.
OrdersList::~OrdersList() {
    for (int i = 0; i < *count; ++i) delete orders[i];
    delete[] orders;
    delete count;
    delete cap;
}

/// @brief Adds an order to the order list.
/// @param order The order to add to the list.
/// @return True if successful, false otherwise.
bool OrdersList::add(Order* order) {
    if (!order || *count >= *cap) return false;
    orders[*count] = order;
    ++(*count);
    return true;
}

/// @brief Removes an order on the order list
/// @param index The specificed index in the order list
/// @return True if successful, false otherwise.
bool OrdersList::remove(int index) {
    if (index < 0 || index >= *count) return false;
    delete orders[index];
    for (int i = index; i < *count - 1; ++i) orders[i] = orders[i + 1];
    orders[*count - 1] = nullptr;
    --(*count);
    return true;
}

/// @brief Moves one order to another location in the list -- shifts the list
/// @param from The index that contains the order that is going to be moved in the list.
/// @param to The index that the order will move to in the list.
/// @return True if the move was successful, false otherwise.
bool OrdersList::move(int from, int to) {
    if (from < 0 || from >= *count || to < 0 || to >= *count) return false;
    if (from == to) return true;
    Order* o = orders[from];
    if (from < to) {
        for (int i = from; i < to; ++i) orders[i] = orders[i + 1];
    } else {
        for (int i = from; i > to; --i) orders[i] = orders[i - 1];
    }
    orders[to] = o;
    return true;
}

int    OrdersList::size()     const { return *count; }
int    OrdersList::capacity() const { return *cap; }

/// @brief Grabs the order in that index in the order list.
/// @param index The index in the order list to get.
/// @return The order that is in the index in the OrderList array.
Order* OrdersList::get(int index) const {
    if (index < 0 || index >= *count) return nullptr;
    return orders[index];
}

/// @brief Stream insertion operator for OrdersList: prints the list header and each order.
/// @param out The output stream object.
/// @param list The OrderList object.
/// @return The string output for each orders in the OrderList object.
std::ostream& operator<<(std::ostream& out, const OrdersList& list) {
    out << "OrdersList (" << *list.count << " / " << *list.cap << " orders):\n";
    for (int i = 0; i < *list.count; ++i) {
        out << "  [" << i << "] " << *list.orders[i];
        if (i + 1 < *list.count) out << "\n";
    }
    return out;
}