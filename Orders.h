// Orders.h
// COMP 345 - Assignment 2 - Part 4 : Orders
// Does NOT #include Player.h or Map.h.
// Player and Territory are pre-declared so the interface is ready for Parts 1 & 2. 
// When those parts exist, only the TODO bodies in Orders.cpp need to be filled in - nothing in this header changes.

#ifndef ORDERS_H
#define ORDERS_H

#include <ostream>
#include <string>

#include "LoggingObserver.h"

class Player;      // Part 2 -  pre-declared
class Territory;   // Part 1 -  pre-declared

// ============================================================
// Order : Abstract base class.
// Part 5: an Order is a Subject and an ILoggable. Every execute() calls
// Notify(this) after recording its effect, so the effect is logged.
// ============================================================
class Order : public Subject, public ILoggable {
public:
    Order(); // Default constructor
    Order(const Order& other); // Copy constructor
    Order& operator=(const Order& other); // Assignment operator
    virtual ~Order(); // Destructor

    virtual bool validate() const = 0;
    virtual void execute() = 0;
    virtual Order* newCopy() const = 0;     // deep-copy helper for OrdersList
    virtual std::string getDescription() const = 0;

    void setEffect(const std::string& e);
    std::string getEffect() const;

    // Part 5: line written in the game log when the order is executed
    std::string stringToLog() const override;

    friend std::ostream& operator<<(std::ostream& out, const Order& o); // Stream insertion operator

protected:
    std::string* effect;   // owned; empty until execute() runs
};

// ============================================================
// Deploy : Place armies on one of the player's territories.
// ============================================================
class Deploy : public Order {
public:
    Deploy(); // Default constructor
    Deploy(Player* player, Territory* target, int armies); // Parameterized constructor
    Deploy(const Deploy& other); // Copy constructor
    Deploy& operator=(const Deploy& other); // Assignment operator
    ~Deploy(); // Destructor

    bool        validate() const;
    void        execute();
    Order*      newCopy() const;
    std::string getDescription() const;

    friend std::ostream& operator<<(std::ostream& out, const Deploy& order); // Stream insertion operator

private:
    Player*    player;   // Part 2 - not owned
    Territory* target;   // Part 1 - not owned
    int*       armies;   // owned
};

// ============================================================
// Advance : Move/attack from an adjacent territory to another.
// ============================================================
class Advance : public Order {
public:
    Advance(); // Default constructor
    Advance(Player* player, Territory* from, Territory* to, int armies); // Parameterized constructor
    Advance(const Advance& other); // Copy constructor
    Advance& operator=(const Advance& other); // Assignment operator 
    ~Advance(); // Destructor

    bool        validate() const;
    void        execute();
    Order*      newCopy() const;
    std::string getDescription() const;

    bool didConquer() const;

    friend std::ostream& operator<<(std::ostream& out, const Advance& order); // Stream insertion operator

private:
    Player*    player;
    Territory* from;
    Territory* to;
    int*       armies;
    bool*      conquered;
};

// ============================================================
// Bomb : Destroy half the armies on an enemy territory.
// ============================================================
class Bomb : public Order {
public:
    Bomb(); // Default constructor
    Bomb(Player* player, Territory* target); // Parameterized constructor
    Bomb(const Bomb& other); // Copy constructor
    Bomb& operator=(const Bomb& other); // Assignment operator 
    ~Bomb(); // Destructor

    bool        validate() const;
    void        execute();
    Order*      newCopy() const;
    std::string getDescription() const;

    friend std::ostream& operator<<(std::ostream& out, const Bomb& order); // Stream insertion operator

private:
    Player*    player;
    Territory* target;
};

// ============================================================
// Blockade : Double the armies on own territory and give it to the Neutral player.
// ============================================================
class Blockade : public Order {
public:
    Blockade(); // Default constructor
    Blockade(Player* player, Territory* target); // Parameterized constructor
    Blockade(const Blockade& other); // Copy constructor
    Blockade& operator=(const Blockade& other); // Assignment operator 
    ~Blockade(); // Destructor

    bool        validate() const;
    void        execute();
    Order*      newCopy() const;
    std::string getDescription() const;

    friend std::ostream& operator<<(std::ostream& out, const Blockade& order); // Stream insertion operator

private:
    Player*    player;
    Territory* target;
};

// ============================================================
// Airlift : Move armies to any territory (no adjacency required).
// ============================================================
class Airlift : public Order {
public:
    Airlift(); // Default constructor
    Airlift(Player* player, Territory* from, Territory* to, int armies); // Parameterized constructor
    Airlift(const Airlift& other); // Copy constructor
    Airlift& operator=(const Airlift& other); // Assignment operator 
    ~Airlift(); // Destructor

    bool        validate() const;
    void        execute();
    Order*      newCopy() const;
    std::string getDescription() const;

    friend std::ostream& operator<<(std::ostream& out, const Airlift& order); // Stream insertion operator

private:
    Player*    player;
    Territory* from;
    Territory* to;
    int*       armies;
};

// ============================================================
// Negotiate : Prevent attacks between two players this turn.
// ============================================================
class Negotiate : public Order {
public:
    Negotiate(); // Default constructor
    Negotiate(Player* player, Player* otherPlayer); // Parameterized constructor
    Negotiate(const Negotiate& other); // Copy constructor
    Negotiate& operator=(const Negotiate& other); // Assignment operator 
    ~Negotiate(); // Destructor

    bool        validate() const;
    void        execute();
    Order*      newCopy() const;
    std::string getDescription() const;

    friend std::ostream& operator<<(std::ostream& out, const Negotiate& order); // Stream insertion operator

private:
    Player* player;
    Player* otherPlayer;
};

// ============================================================
// OrdersList : Owns a collection of Order*  (fixed capacity).
// Part 5: an OrdersList is a Subject and an ILoggable. addOrder() calls
// Notify(this), so every order put in the list is logged.
// ============================================================
class OrdersList : public Subject, public ILoggable {
public:
    static const int DEFAULT_CAPACITY = 32; // Max orders list -- can be adjusted for better memory management.

    OrdersList(); // Default constructor
    OrdersList(const OrdersList& other); // Copy constructor
    OrdersList& operator=(const OrdersList& other); // Assignment operator overload
    ~OrdersList();

    bool   addOrder(Order* order);   // Adds an order at the end of the list and notifies the log observer (Part 5)
    bool   add(Order* order);        // Same as addOrder(), kept so existing calls keep working
    bool   remove(int index);
    bool   move(int from, int to);
    int    size() const;
    int    capacity() const;
    Order* get(int index) const;

    // Part 5: line written in the game log when an order is added
    std::string stringToLog() const override;

    friend std::ostream& operator<<(std::ostream& out, const OrdersList& list); // Stream insertion operator

private:
    Order** orders;   // owned static array of owned Order*
    int*    count;    // owned
    int*    cap;      // owned
};

#endif // ORDERS_H