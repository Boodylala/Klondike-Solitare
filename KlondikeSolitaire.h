#ifndef KLONDIKESOLITAIRE_H
#define KLONDIKESOLITAIRE_H

#include "Card.h"
#include "Stack.h"
#include "CardList.h"

// Move structure to store move history
struct Move {
    enum MoveType {
        DRAW_STOCK,
        WASTE_TO_TABLEAU,
        WASTE_TO_FOUNDATION,
        TABLEAU_TO_FOUNDATION,
        TABLEAU_TO_TABLEAU,
        TABLEAU_SEQUENCE,
        FOUNDATION_TO_TABLEAU,
        REFILL_STOCK
    };

    MoveType type;
    int fromIndex;
    int toIndex;
    int cardCount;
    int scoreChange;
    bool flippedCard;
};

class KlondikeSolitaire {
private:
    Card* deck[52];
    Stack<Card*> stock;
    Stack<Card*> waste;
    Stack<Card*> foundations[4];
    CardList tableau[7];
    int score;

    // Undo functionality
    Stack<Move> moveHistory;
    static const int MAX_UNDO = 50;

    void createDeck();
    void shuffleDeck();
    void dealCards();
    bool canPlaceOnTableau(Card* card, Card* target);
    bool canPlaceOnFoundation(Card* card, int foundIndex);
    void recordMove(Move move);

public:
    KlondikeSolitaire();
    ~KlondikeSolitaire();

    void drawFromStock();
    bool moveWasteToTableau(int tableauIndex);
    bool moveWasteToFoundation(int foundIndex);
    bool moveTableauToFoundation(int tableauIndex, int foundIndex);
    bool moveTableauToTableau(int fromIndex, int toIndex);
    bool moveTableauSequence(int fromIndex, int toIndex, int startCardIndex);
    bool moveFoundationToTableau(int foundIndex, int tableauIndex);
    bool undo();
    bool canUndo() const;
    bool isGameWon();

    // GUI support methods
    Card* getWasteTop();
    Card* getFoundationTop(int index);
    Card* getTableauCard(int pile, int index);
    int getTableauSize(int pile);
    bool isStockEmpty();
    int getScore();
};

#endif // KLONDIKESOLITAIRE_H
