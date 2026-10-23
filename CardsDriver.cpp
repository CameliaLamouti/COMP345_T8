// CardsDriver.cpp
// COMP 345 - Assignment 1 - Part 4 driver.
// Creates a deck holding cards of every kind, fills a player's hand by
// drawing cards from the deck, then plays every card in the hand. Each play
// adds an order to the player's OrdersList, removes the card from the hand
// and puts it back in the deck.
//
// How to use:
// 1) Compile: g++ -std=c++17 Cards.cpp CardsDriver.cpp Player.cpp Orders.cpp Map.cpp -o cards_driver
// 2) Run:     ./cards_driver

#include "Cards.h"
#include "Orders.h"
#include "Player.h"

#include <iostream>

// Number of cards the player draws from the deck into their hand.
static const int CARDS_TO_DRAW = 5;

/**
 * Demonstrates the Card, Deck and Hand classes together with Player,
 * Order and OrdersList.
 */
int main() {
    std::cout << "=== Part 4: Cards deck/hand demo ===\n\n";

    Deck* deck = new Deck();
    Player* player = new Player("Alice");
    Hand* hand = player->getHand();

    std::cout << "--- 1) A deck with cards of every kind ---\n";
    std::cout << "Deck: " << *deck << "\n";
    std::cout << "Alice's hand: " << *hand << "\n\n";

    std::cout << "--- 2) draw() " << CARDS_TO_DRAW << " random cards into Alice's hand ---\n";
    for (int i = 0; i < CARDS_TO_DRAW; ++i) {
        Card* card = deck->draw(hand);
        std::cout << "  drew " << *card << " (" << deck->size() << " cards left in the deck)\n";
    }
    std::cout << "Deck: " << *deck << "\n";
    std::cout << "Alice's hand: " << *hand << "\n\n";

    std::cout << "--- 3) play() every card in Alice's hand ---\n";
    while (!hand->isEmpty()) {
        Card* card = hand->getCard(0);
        std::cout << "  playing " << *card << " -> ";
        if (!card->play(player, deck)) {
            std::cout << "could not be played\n";
            break;
        }
        OrdersList* orders = player->getOrders();
        std::cout << "order added: " << *orders->get(orders->size() - 1)
                  << " | card back in deck (" << deck->size() << " cards)\n";
    }
    std::cout << "Deck: " << *deck << "\n";
    std::cout << "Alice's hand: " << *hand << "\n";
    std::cout << "Alice's " << *player->getOrders() << "\n\n";

    std::cout << "--- 4) Invalid uses are rejected ---\n";
    Card notInHand(CardType::Bomb);
    std::cout << "  play() a card that is not in Alice's hand: "
              << (notInHand.play(player, deck) ? "accepted" : "rejected") << "\n";
    Deck emptyDeck(0);
    std::cout << "  draw() from an empty deck returns: "
              << (emptyDeck.draw(hand) == nullptr ? "nullptr" : "a card") << "\n\n";

    std::cout << "--- 5) Copy constructor and assignment operator make deep copies ---\n";
    deck->draw(hand);
    deck->draw(hand);
    Hand handCopy(*hand);
    Hand handAssigned;
    handAssigned = *hand;
    Deck deckCopy(*deck);
    Deck deckAssigned(1);
    deckAssigned = *deck;
    Card cardCopy(*hand->getCard(0));
    Card cardAssigned(CardType::Diplomacy);
    cardAssigned = *hand->getCard(1);

    std::cout << "Alice's hand:  " << *hand << "\n";
    std::cout << "Original deck: " << *deck << "\n";

    delete player;   // also deletes Alice's hand and orders
    delete deck;
    player = nullptr;
    deck = nullptr;
    hand = nullptr;

    std::cout << "After deleting Alice and the original deck, the copies are intact:\n";
    std::cout << "  copied hand:   " << handCopy << "\n";
    std::cout << "  assigned hand: " << handAssigned << "\n";
    std::cout << "  copied deck:   " << deckCopy << "\n";
    std::cout << "  assigned deck: " << deckAssigned << "\n";
    std::cout << "  copied card:   " << cardCopy << "\n";
    std::cout << "  assigned card: " << cardAssigned << "\n";

    return 0;
}
