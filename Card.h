#ifndef CARD_H
#define CARD_H

class Card {
public:
    enum Suit { HEARTS, DIAMONDS, CLUBS, SPADES };
    enum Rank { ACE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING };

private:
    Suit suit;
    Rank rank;
    bool faceUp;

public:
    Card(Suit s = HEARTS, Rank r = ACE);

    Suit getSuit() const;
    Rank getRank() const;
    bool isFaceUp() const;
    void flip();
    void setFaceUp(bool up);

    bool isRed() const;
    bool isBlack() const;
};

#endif
