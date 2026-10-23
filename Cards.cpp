// Cards.cpp
// COMP 345 - Assignment 1 - Part 4: Cards deck/hand
// Implementation of the Card, Deck and Hand classes declared in Cards.h.

#include "Cards.h"

#include "Orders.h"
#include "Player.h"

#include <algorithm>
#include <cctype>
#include <random>
#include <stdexcept>

// ---------------------------- Helper Functions ---------------------------- //

// Number of armies given by a reinforcement card (see the project description).
static const int REINFORCEMENT_ARMIES = 5;

// Every card type, in the order they are listed in the assignment.
static const CardType ALL_CARD_TYPES[] = {
    CardType::Bomb, CardType::Reinforcement, CardType::Blockade,
    CardType::Airlift, CardType::Diplomacy
};

/**
 * Returns the random number engine used by Deck::draw().
 * It is created and seeded only once, the first time it is needed.
 */
static std::mt19937& randomEngine() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

/**
 * Converts a card name such as "bomb" or "Airlift" into its CardType.
 * Throws std::invalid_argument if the name is not one of the five card types.
 */
static CardType cardTypeFromString(const std::string& name) {
    std::string lowercase = name;
    for (char& c : lowercase) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    for (CardType type : ALL_CARD_TYPES) {
        if (cardTypeToString(type) == lowercase) {
            return type;
        }
    }
    throw std::invalid_argument("Unknown card type: " + name);
}

/**
 * Deletes every card in a list, then empties the list.
 */
static void deleteCards(std::vector<Card*>& cards) {
    for (Card* card : cards) {
        delete card;
    }
    cards.clear();
}

/**
 * Appends a new copy of every card of source to destination (deep copy).
 */
static void copyCards(const std::vector<Card*>& source, std::vector<Card*>& destination) {
    for (const Card* card : source) {
        destination.push_back(new Card(*card));
    }
}

/**
 * Returns the lowercase name of a card type, e.g. "bomb".
 */
std::string cardTypeToString(CardType type) {
    switch (type) {
        case CardType::Bomb:          return "bomb";
        case CardType::Reinforcement: return "reinforcement";
        case CardType::Blockade:      return "blockade";
        case CardType::Airlift:       return "airlift";
        case CardType::Diplomacy:     return "diplomacy";
    }
    return "unknown";
}

// ---------------------------------- Card ---------------------------------- //

/**
 * Creates a card of the given type.
 */
Card::Card(CardType type) : type(new CardType(type)) {}

/**
 * Creates a card from its name ("bomb", "reinforcement", "blockade",
 * "airlift" or "diplomacy"). Throws std::invalid_argument for any other name.
 */
Card::Card(const std::string& typeName) : type(new CardType(cardTypeFromString(typeName))) {}

/**
 * Copy constructor: the new card gets its own copy of the type.
 */
Card::Card(const Card& other) : type(new CardType(*other.type)) {}

/**
 * Assignment operator: copies the type of the other card.
 */
Card& Card::operator=(const Card& other) {
    if (this != &other) {
        *type = *other.type;
    }
    return *this;
}

/**
 * Destructor: frees the type.
 */
Card::~Card() {
    delete type;
}

/**
 * Returns the type of the card.
 */
CardType Card::getType() const {
    return *type;
}

/**
 * Returns the type of the card as text, e.g. "airlift".
 */
std::string Card::getTypeName() const {
    return cardTypeToString(*type);
}

/**
 * Plays the card for the given player:
 *   1. creates the order that matches the card's type,
 *   2. adds that order to the player's list of orders,
 *   3. removes the card from the player's hand,
 *   4. puts the card back in the deck.
 * The card object itself moves from the hand to the deck; it is not copied.
 * Returns false and changes nothing if the player or deck is missing, if the
 * card is not in the player's hand, or if the player's orders list is full.
 */
bool Card::play(Player* player, Deck* deck) {
    if (player == nullptr || deck == nullptr) {
        return false;
    }
    Hand* hand = player->getHand();
    if (!hand->contains(this)) {
        return false;
    }

    Order* order = createOrder(player);
    if (!player->getOrders()->add(order)) {
        delete order;   // the list did not take the order, so it is freed here
        return false;
    }

    hand->removeCard(this);
    deck->addCard(this);
    return true;
}

/**
 * Creates the order that matches the card's type:
 *   bomb -> Bomb, reinforcement -> Deploy of 5 armies, blockade -> Blockade,
 *   airlift -> Airlift, diplomacy -> Negotiate.
 * Only the issuing player is known when the card is played; the target
 * territories, the opponent and the airlift army count are chosen later by
 * the player's strategy, so they are left empty for now.
 */
Order* Card::createOrder(Player* player) const {
    switch (*type) {
        case CardType::Bomb:          return new Bomb(player, nullptr);
        case CardType::Reinforcement: return new Deploy(player, nullptr, REINFORCEMENT_ARMIES);
        case CardType::Blockade:      return new Blockade(player, nullptr);
        case CardType::Airlift:       return new Airlift(player, nullptr, nullptr, 0);
        case CardType::Diplomacy:     return new Negotiate(player, nullptr);
    }
    return nullptr;
}

/**
 * Stream insertion operator: prints the card's type, e.g. "bomb".
 */
std::ostream& operator<<(std::ostream& out, const Card& card) {
    out << card.getTypeName();
    return out;
}

// ---------------------------------- Deck ---------------------------------- //

/**
 * Creates a deck with DEFAULT_CARDS_PER_TYPE cards of every type.
 */
Deck::Deck() : Deck(DEFAULT_CARDS_PER_TYPE) {}

/**
 * Creates a deck with cardsPerType cards of every type.
 * A value of 0 or less creates an empty deck.
 */
Deck::Deck(int cardsPerType) : cards(new std::vector<Card*>()) {
    for (CardType type : ALL_CARD_TYPES) {
        for (int i = 0; i < cardsPerType; ++i) {
            cards->push_back(new Card(type));
        }
    }
}

/**
 * Copy constructor: the new deck gets its own copy of every card.
 */
Deck::Deck(const Deck& other) : cards(new std::vector<Card*>()) {
    copyCards(*other.cards, *cards);
}

/**
 * Assignment operator: deletes this deck's cards, then copies every card
 * of the other deck.
 */
Deck& Deck::operator=(const Deck& other) {
    if (this != &other) {
        deleteCards(*cards);
        copyCards(*other.cards, *cards);
    }
    return *this;
}

/**
 * Destructor: deletes the cards still in the deck, then the list itself.
 * Cards that are in a hand at this point belong to that hand.
 */
Deck::~Deck() {
    deleteCards(*cards);
    delete cards;
}

/**
 * Picks a card at random among the cards remaining in the deck, removes it
 * from the deck, places it in the given hand and returns it.
 * Returns nullptr (and changes nothing) if the deck is empty or hand is nullptr.
 */
Card* Deck::draw(Hand* hand) {
    if (hand == nullptr || cards->empty()) {
        return nullptr;
    }
    std::uniform_int_distribution<std::size_t> pick(0, cards->size() - 1);
    std::size_t index = pick(randomEngine());

    Card* card = (*cards)[index];
    cards->erase(cards->begin() + static_cast<std::ptrdiff_t>(index));
    hand->addCard(card);
    return card;
}

/**
 * Puts a card in the deck, which becomes its owner.
 * Returns false if card is nullptr.
 */
bool Deck::addCard(Card* card) {
    if (card == nullptr) {
        return false;
    }
    cards->push_back(card);
    return true;
}

/**
 * Returns the number of cards in the deck.
 */
int Deck::size() const {
    return static_cast<int>(cards->size());
}

/**
 * Tells whether the deck has no cards left.
 */
bool Deck::isEmpty() const {
    return cards->empty();
}

/**
 * Returns how many cards of the given type are in the deck.
 */
int Deck::countOfType(CardType type) const {
    int count = 0;
    for (const Card* card : *cards) {
        if (card->getType() == type) {
            ++count;
        }
    }
    return count;
}

/**
 * Stream insertion operator: prints the number of cards and how many of each
 * type the deck holds, e.g. "15 card(s): bomb x3, reinforcement x3, ...".
 */
std::ostream& operator<<(std::ostream& out, const Deck& deck) {
    out << deck.size() << " card(s): ";
    bool first = true;
    for (CardType type : ALL_CARD_TYPES) {
        if (!first) {
            out << ", ";
        }
        out << cardTypeToString(type) << " x" << deck.countOfType(type);
        first = false;
    }
    return out;
}

// ---------------------------------- Hand ---------------------------------- //

/**
 * Creates an empty hand.
 */
Hand::Hand() : cards(new std::vector<Card*>()) {}

/**
 * Copy constructor: the new hand gets its own copy of every card.
 */
Hand::Hand(const Hand& other) : cards(new std::vector<Card*>()) {
    copyCards(*other.cards, *cards);
}

/**
 * Assignment operator: deletes this hand's cards, then copies every card
 * of the other hand.
 */
Hand& Hand::operator=(const Hand& other) {
    if (this != &other) {
        deleteCards(*cards);
        copyCards(*other.cards, *cards);
    }
    return *this;
}

/**
 * Destructor: deletes the cards still in the hand, then the list itself.
 */
Hand::~Hand() {
    deleteCards(*cards);
    delete cards;
}

/**
 * Puts a card in the hand, which becomes its owner.
 * Returns false if card is nullptr or is already in the hand.
 */
bool Hand::addCard(Card* card) {
    if (card == nullptr || contains(card)) {
        return false;
    }
    cards->push_back(card);
    return true;
}

/**
 * Takes a card out of the hand without deleting it: whoever removes it
 * (e.g. Card::play(), which gives it back to the deck) becomes its owner.
 * Returns false if the card is not in the hand.
 */
bool Hand::removeCard(Card* card) {
    std::vector<Card*>::iterator position = std::find(cards->begin(), cards->end(), card);
    if (position == cards->end()) {
        return false;
    }
    cards->erase(position);
    return true;
}

/**
 * Tells whether this exact card object is in the hand.
 */
bool Hand::contains(const Card* card) const {
    return std::find(cards->begin(), cards->end(), card) != cards->end();
}

/**
 * Returns the number of cards in the hand.
 */
int Hand::size() const {
    return static_cast<int>(cards->size());
}

/**
 * Tells whether the hand has no cards.
 */
bool Hand::isEmpty() const {
    return cards->empty();
}

/**
 * Returns the card at the given position, or nullptr if the position is
 * out of range.
 */
Card* Hand::getCard(int index) const {
    if (index < 0 || index >= size()) {
        return nullptr;
    }
    return (*cards)[index];
}

/**
 * Stream insertion operator: prints the number of cards and their types,
 * e.g. "3 card(s): bomb, airlift, diplomacy".
 */
std::ostream& operator<<(std::ostream& out, const Hand& hand) {
    out << hand.size() << " card(s)";
    for (int i = 0; i < hand.size(); ++i) {
        out << (i == 0 ? ": " : ", ") << *hand.getCard(i);
    }
    return out;
}
