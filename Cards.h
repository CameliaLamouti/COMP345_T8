// Cards.h
// COMP 345 - Assignment 1 - Part 4: Cards deck/hand
//
// A Deck holds a finite collection of Warzone cards. A player draws a random
// card from the Deck into their Hand. Playing a card creates the matching
// special order, adds it to the player's OrdersList, removes the card from the
// Hand and puts it back in the Deck.
//
// Ownership: a Card is owned by whichever container (Deck or Hand) currently
// holds it. draw() and play() move the same Card object from one container to
// the other, so a card is never lost or deleted twice while it changes hands.

#ifndef CARDS_H
#define CARDS_H

#include <ostream>
#include <string>
#include <vector>

class Player;   // Part 2 - the player who draws and plays cards
class Order;    // Part 3 - the order created when a card is played
class Deck;
class Hand;

// The five kinds of Warzone cards.
enum class CardType { Bomb, Reinforcement, Blockade, Airlift, Diplomacy };

// Returns the lowercase name of a card type, e.g. "bomb".
std::string cardTypeToString(CardType type);

// ============================================================
// Card : a single Warzone card of one of the five types.
// ============================================================
class Card {
public:
    explicit Card(CardType type);                // Creates a card of the given type
    explicit Card(const std::string& typeName);  // Creates a card from its name ("bomb", "airlift", ...); throws std::invalid_argument for an unknown name
    Card(const Card& other);                     // Copy constructor
    Card& operator=(const Card& other);          // Assignment operator
    ~Card();                                     // Destructor

    CardType    getType() const;       // Returns the type of the card
    std::string getTypeName() const;   // Returns the type of the card as text

    // Plays the card: creates the matching order, adds it to the player's list
    // of orders, removes the card from the player's hand and puts it back in
    // the deck. Returns false and changes nothing if the card is not in the
    // player's hand or the player's list of orders is full.
    bool play(Player* player, Deck* deck);

    friend std::ostream& operator<<(std::ostream& out, const Card& card); // Stream insertion operator

private:
    CardType* type;   // owned

    Order* createOrder(Player* player) const;   // Creates the order that matches the card's type
};

// ============================================================
// Deck : a finite collection of cards that players draw from.
// ============================================================
class Deck {
public:
    static const int DEFAULT_CARDS_PER_TYPE = 3;   // Copies of each card type in a default deck

    Deck();                              // Creates a deck with DEFAULT_CARDS_PER_TYPE cards of every type
    explicit Deck(int cardsPerType);     // Creates a deck with cardsPerType cards of every type
    Deck(const Deck& other);             // Copy constructor (deep copy of every card)
    Deck& operator=(const Deck& other);  // Assignment operator (deep copy of every card)
    ~Deck();                             // Destructor: deletes the cards still in the deck

    // Removes a random card from the deck, places it in the given hand and
    // returns it. Returns nullptr if the deck is empty or hand is nullptr.
    Card* draw(Hand* hand);

    bool addCard(Card* card);               // Puts a card in the deck, which takes ownership of it
    int  size() const;                      // Returns the number of cards in the deck
    bool isEmpty() const;                   // Tells whether the deck has no cards left
    int  countOfType(CardType type) const;  // Returns how many cards of the given type are in the deck

    friend std::ostream& operator<<(std::ostream& out, const Deck& deck); // Stream insertion operator

private:
    std::vector<Card*>* cards;   // owned, along with every Card it points to
};

// ============================================================
// Hand : the finite collection of cards held by one player.
// ============================================================
class Hand {
public:
    Hand();                              // Creates an empty hand
    Hand(const Hand& other);             // Copy constructor (deep copy of every card)
    Hand& operator=(const Hand& other);  // Assignment operator (deep copy of every card)
    ~Hand();                             // Destructor: deletes the cards still in the hand

    bool  addCard(Card* card);               // Puts a card in the hand, which takes ownership of it
    bool  removeCard(Card* card);            // Takes a card out of the hand without deleting it (ownership goes to the caller)
    bool  contains(const Card* card) const;  // Tells whether this exact card is in the hand
    int   size() const;                      // Returns the number of cards in the hand
    bool  isEmpty() const;                   // Tells whether the hand has no cards
    Card* getCard(int index) const;          // Returns the card at a position, or nullptr if out of range

    friend std::ostream& operator<<(std::ostream& out, const Hand& hand); // Stream insertion operator

private:
    std::vector<Card*>* cards;   // owned, along with every Card it points to
};

#endif // CARDS_H
