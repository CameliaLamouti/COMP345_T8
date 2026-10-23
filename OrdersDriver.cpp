// OrdersDriver.cpp
// COMP 345 - Assignment 1 - Part 3 driver.
// Creates one of every order kind, puts them in an OrdersList,
// demonstrates move(), remove(), validate(), execute(), and operator<<.
// Since Parts 1 & 2 are not present, Player* and Territory* arguments
// are passed as nullptr; validate() still does its basic checks
// (e.g. armies > 0) and execute() only records a placeholder effect.
//
// How to use:
// 1) Compile: g++ Orders.cpp OrdersDriver.cpp -o orders_driver
// 2) Run:     ./orders_driver

#include "Orders.h"
#include <iostream>

int main() {
    std::cout << "=== Part 3: OrdersList demo ===\n\n";

    OrdersList* list = new OrdersList();

    // One of each order kind (nullptr for Part 1/2 pointers)
    list->add(new Deploy   (nullptr, nullptr, 5));
    list->add(new Advance  (nullptr, nullptr, nullptr, 3));
    list->add(new Bomb     (nullptr, nullptr));
    list->add(new Blockade (nullptr, nullptr));
    list->add(new Airlift  (nullptr, nullptr, nullptr, 4));
    list->add(new Negotiate(nullptr, nullptr));

    // An intentionally invalid order (armies = 0) to show invalid path
    list->add(new Deploy   (nullptr, nullptr, 0));

    std::cout << "--- Initial list (7 orders) ---\n";
    std::cout << *list << "\n\n";

    std::cout << "--- validate() each order ---\n";
    for (int i = 0; i < list->size(); ++i)
        std::cout << "  [" << i << "] valid? "
                  << (list->get(i)->validate() ? "yes" : "no") << "\n";
    std::cout << "\n";

    std::cout << "--- execute() each order (actions are placeholders) ---\n";
    for (int i = 0; i < list->size(); ++i)
        list->get(i)->execute();
    std::cout << *list << "\n\n";

    std::cout << "--- move(0, 3) : move Deploy from front to index 3 ---\n";
    list->move(0, 3);
    std::cout << *list << "\n\n";

    std::cout << "--- remove(2) : delete the order at index 2 ---\n";
    list->remove(2);
    std::cout << *list << "\n\n";

    std::cout << "--- copy constructor : deep copy survives delete of original ---\n";
    OrdersList copy(*list);
    delete list;                    // original gone -- copy must be intact
    std::cout << copy << "\n";

    return 0;
}