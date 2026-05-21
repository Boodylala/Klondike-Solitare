#include "Card.h"

Card::Card(Suit s, Rank r) : suit(s), rank(r), faceUp(false) {}

Card::Suit Card::getSuit() const { return suit; }
Card::Rank Card::getRank() const { return rank; }
bool Card::isFaceUp() const { return faceUp; }
void Card::flip() { faceUp = !faceUp; }
void Card::setFaceUp(bool up) { faceUp = up; }

bool Card::isRed() const { return suit == HEARTS || suit == DIAMONDS; }
bool Card::isBlack() const { return !isRed(); }
