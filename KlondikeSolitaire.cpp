#include "KlondikeSolitaire.h"
#include <cstdlib>
#include <ctime>

KlondikeSolitaire::KlondikeSolitaire() : score(0) {
    srand(time(0));
    createDeck();
    shuffleDeck();
    dealCards();
}

KlondikeSolitaire::~KlondikeSolitaire() {
    for (int i = 0; i < 52; i++) {
        delete deck[i];
    }
}

void KlondikeSolitaire::createDeck() {
    int index = 0;
    for (int s = Card::HEARTS; s <= Card::SPADES; s++) {
        for (int r = Card::ACE; r <= Card::KING; r++) {
            deck[index++] = new Card(static_cast<Card::Suit>(s),
                                     static_cast<Card::Rank>(r));
        }
    }
}

void KlondikeSolitaire::shuffleDeck() {
    for (int i = 51; i > 0; i--) {
        int j = rand() % (i + 1);
        Card* temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

void KlondikeSolitaire::dealCards() {
    int deckIndex = 0;

    for (int pile = 0; pile < 7; pile++) {
        for (int card = 0; card <= pile; card++) {
            Card* c = deck[deckIndex++];
            if (card == pile) {
                c->setFaceUp(true);
            }
            tableau[pile].addCard(c);
        }
    }

    for (int i = deckIndex; i < 52; i++) {
        stock.push(deck[i]);
    }
}

void KlondikeSolitaire::recordMove(Move move) {
    if (moveHistory.size() >= MAX_UNDO) {
        // Limit to MAX_UNDO moves
    }
    moveHistory.push(move);
}

void KlondikeSolitaire::drawFromStock() {
    if (stock.isEmpty()) {
        int refillCount = 0;
        while (!waste.isEmpty()) {
            Card* card = waste.pop();
            card->setFaceUp(false);
            stock.push(card);
            refillCount++;
        }

        Move move;
        move.type = Move::REFILL_STOCK;
        move.cardCount = refillCount;
        move.scoreChange = 0;
        move.flippedCard = false;
        recordMove(move);
    } else {
        Card* card = stock.pop();
        card->setFaceUp(true);
        waste.push(card);

        Move move;
        move.type = Move::DRAW_STOCK;
        move.scoreChange = 0;
        move.flippedCard = false;
        recordMove(move);
    }
}

bool KlondikeSolitaire::canPlaceOnTableau(Card* card, Card* target) {
    if (!target) {
        return card->getRank() == Card::KING;
    }
    return (card->isRed() != target->isRed()) &&
           (card->getRank() == target->getRank() - 1);
}

bool KlondikeSolitaire::canPlaceOnFoundation(Card* card, int foundIndex) {
    if (foundations[foundIndex].isEmpty()) {
        return card->getRank() == Card::ACE;
    }
    Card* topCard = foundations[foundIndex].peek();
    return (card->getSuit() == topCard->getSuit()) &&
           (card->getRank() == topCard->getRank() + 1);
}

bool KlondikeSolitaire::moveWasteToTableau(int tableauIndex) {
    if (waste.isEmpty() || tableauIndex < 0 || tableauIndex > 6) {
        return false;
    }

    Card* card = waste.peek();
    Card* target = tableau[tableauIndex].getLastCard();

    if (canPlaceOnTableau(card, target)) {
        waste.pop();
        tableau[tableauIndex].addCard(card);
        score += 5;

        Move move;
        move.type = Move::WASTE_TO_TABLEAU;
        move.toIndex = tableauIndex;
        move.scoreChange = 5;
        move.flippedCard = false;
        recordMove(move);

        return true;
    }
    return false;
}

bool KlondikeSolitaire::moveWasteToFoundation(int foundIndex) {
    if (waste.isEmpty() || foundIndex < 0 || foundIndex > 3) {
        return false;
    }

    Card* card = waste.peek();
    if (canPlaceOnFoundation(card, foundIndex)) {
        waste.pop();
        foundations[foundIndex].push(card);
        score += 10;

        Move move;
        move.type = Move::WASTE_TO_FOUNDATION;
        move.toIndex = foundIndex;
        move.scoreChange = 10;
        move.flippedCard = false;
        recordMove(move);

        return true;
    }
    return false;
}

bool KlondikeSolitaire::moveTableauToFoundation(int tableauIndex, int foundIndex) {
    if (tableauIndex < 0 || tableauIndex > 6 || foundIndex < 0 || foundIndex > 3) {
        return false;
    }

    Card* card = tableau[tableauIndex].getLastCard();
    if (!card || !card->isFaceUp()) return false;

    if (canPlaceOnFoundation(card, foundIndex)) {
        tableau[tableauIndex].removeLastCard();
        foundations[foundIndex].push(card);

        bool flipped = false;
        Card* nextCard = tableau[tableauIndex].getLastCard();
        if (nextCard && !nextCard->isFaceUp()) {
            nextCard->setFaceUp(true);
            flipped = true;
        }

        score += 10;

        Move move;
        move.type = Move::TABLEAU_TO_FOUNDATION;
        move.fromIndex = tableauIndex;
        move.toIndex = foundIndex;
        move.scoreChange = 10;
        move.flippedCard = flipped;
        recordMove(move);

        return true;
    }
    return false;
}

bool KlondikeSolitaire::moveTableauToTableau(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex > 6 || toIndex < 0 || toIndex > 6) {
        return false;
    }

    Card* card = tableau[fromIndex].getLastCard();
    if (!card || !card->isFaceUp()) return false;

    Card* target = tableau[toIndex].getLastCard();

    if (canPlaceOnTableau(card, target)) {
        tableau[fromIndex].removeLastCard();
        tableau[toIndex].addCard(card);

        bool flipped = false;
        Card* nextCard = tableau[fromIndex].getLastCard();
        if (nextCard && !nextCard->isFaceUp()) {
            nextCard->setFaceUp(true);
            flipped = true;
        }

        Move move;
        move.type = Move::TABLEAU_TO_TABLEAU;
        move.fromIndex = fromIndex;
        move.toIndex = toIndex;
        move.cardCount = 1;
        move.scoreChange = 0;
        move.flippedCard = flipped;
        recordMove(move);

        return true;
    }
    return false;
}

bool KlondikeSolitaire::moveTableauSequence(int fromIndex, int toIndex, int startCardIndex) {
    if (fromIndex < 0 || fromIndex > 6 || toIndex < 0 || toIndex > 6) {
        return false;
    }

    int cardCount = tableau[fromIndex].size();
    if (startCardIndex < 0 || startCardIndex >= cardCount) return false;

    Card* firstCard = tableau[fromIndex].getCardAt(startCardIndex);
    if (!firstCard || !firstCard->isFaceUp()) return false;

    Card* target = tableau[toIndex].getLastCard();
    if (!canPlaceOnTableau(firstCard, target)) return false;

    Card* cardsToMove[13];
    int moveCount = 0;

    for (int i = startCardIndex; i < cardCount; i++) {
        cardsToMove[moveCount++] = tableau[fromIndex].getLastCard();
        tableau[fromIndex].removeLastCard();
    }

    for (int i = moveCount - 1; i >= 0; i--) {
        tableau[toIndex].addCard(cardsToMove[i]);
    }

    bool flipped = false;
    Card* nextCard = tableau[fromIndex].getLastCard();
    if (nextCard && !nextCard->isFaceUp()) {
        nextCard->setFaceUp(true);
        flipped = true;
    }

    Move move;
    move.type = Move::TABLEAU_SEQUENCE;
    move.fromIndex = fromIndex;
    move.toIndex = toIndex;
    move.cardCount = moveCount;
    move.scoreChange = 0;
    move.flippedCard = flipped;
    recordMove(move);

    return true;
}

bool KlondikeSolitaire::moveFoundationToTableau(int foundIndex, int tableauIndex) {
    if (foundIndex < 0 || foundIndex > 3 || tableauIndex < 0 || tableauIndex > 6) {
        return false;
    }

    if (foundations[foundIndex].isEmpty()) return false;

    Card* card = foundations[foundIndex].peek();
    Card* target = tableau[tableauIndex].getLastCard();

    if (canPlaceOnTableau(card, target)) {
        foundations[foundIndex].pop();
        tableau[tableauIndex].addCard(card);
        score -= 10;

        Move move;
        move.type = Move::FOUNDATION_TO_TABLEAU;
        move.fromIndex = foundIndex;
        move.toIndex = tableauIndex;
        move.scoreChange = -10;
        move.flippedCard = false;
        recordMove(move);

        return true;
    }
    return false;
}

bool KlondikeSolitaire::canUndo() const {
    return !moveHistory.isEmpty();
}

bool KlondikeSolitaire::undo() {
    if (moveHistory.isEmpty()) return false;

    Move lastMove = moveHistory.pop();

    switch(lastMove.type) {
    case Move::DRAW_STOCK:
        if (!waste.isEmpty()) {
            Card* card = waste.pop();
            card->setFaceUp(false);
            stock.push(card);
        }
        break;

    case Move::REFILL_STOCK:
        for (int i = 0; i < lastMove.cardCount; i++) {
            if (!stock.isEmpty()) {
                Card* card = stock.pop();
                card->setFaceUp(true);
                waste.push(card);
            }
        }
        break;

    case Move::WASTE_TO_TABLEAU:
        if (!tableau[lastMove.toIndex].isEmpty()) {
            Card* card = tableau[lastMove.toIndex].removeLastCard();
            waste.push(card);
            score -= lastMove.scoreChange;
        }
        break;

    case Move::WASTE_TO_FOUNDATION:
        if (!foundations[lastMove.toIndex].isEmpty()) {
            Card* card = foundations[lastMove.toIndex].pop();
            waste.push(card);
            score -= lastMove.scoreChange;
        }
        break;

    case Move::TABLEAU_TO_FOUNDATION:
        if (!foundations[lastMove.toIndex].isEmpty()) {
            Card* card = foundations[lastMove.toIndex].pop();
            tableau[lastMove.fromIndex].addCard(card);

            if (lastMove.flippedCard) {
                int size = tableau[lastMove.fromIndex].size();
                if (size > 1) {
                    Card* cardToFlip = tableau[lastMove.fromIndex].getCardAt(size - 2);
                    if (cardToFlip) cardToFlip->setFaceUp(false);
                }
            }
            score -= lastMove.scoreChange;
        }
        break;

    case Move::TABLEAU_TO_TABLEAU:
        if (!tableau[lastMove.toIndex].isEmpty()) {
            Card* card = tableau[lastMove.toIndex].removeLastCard();
            tableau[lastMove.fromIndex].addCard(card);

            if (lastMove.flippedCard) {
                int size = tableau[lastMove.fromIndex].size();
                if (size > 1) {
                    Card* cardToFlip = tableau[lastMove.fromIndex].getCardAt(size - 2);
                    if (cardToFlip) cardToFlip->setFaceUp(false);
                }
            }
        }
        break;

    case Move::TABLEAU_SEQUENCE:
    {
        Card* cardsToMoveBack[13];
        for (int i = 0; i < lastMove.cardCount; i++) {
            if (!tableau[lastMove.toIndex].isEmpty()) {
                cardsToMoveBack[i] = tableau[lastMove.toIndex].removeLastCard();
            }
        }

        for (int i = lastMove.cardCount - 1; i >= 0; i--) {
            tableau[lastMove.fromIndex].addCard(cardsToMoveBack[i]);
        }

        if (lastMove.flippedCard) {
            int size = tableau[lastMove.fromIndex].size();
            if (size > lastMove.cardCount) {
                Card* cardToFlip = tableau[lastMove.fromIndex].getCardAt(size - lastMove.cardCount - 1);
                if (cardToFlip) cardToFlip->setFaceUp(false);
            }
        }
    }
    break;

    case Move::FOUNDATION_TO_TABLEAU:
        if (!tableau[lastMove.toIndex].isEmpty()) {
            Card* card = tableau[lastMove.toIndex].removeLastCard();
            foundations[lastMove.fromIndex].push(card);
            score -= lastMove.scoreChange;
        }
        break;
    }

    return true;
}

bool KlondikeSolitaire::isGameWon() {
    for (int i = 0; i < 4; i++) {
        if (foundations[i].size() != 13) {
            return false;
        }
    }
    return true;
}

Card* KlondikeSolitaire::getWasteTop() {
    return waste.isEmpty() ? nullptr : waste.peek();
}

Card* KlondikeSolitaire::getFoundationTop(int index) {
    if (index < 0 || index > 3) return nullptr;
    return foundations[index].isEmpty() ? nullptr : foundations[index].peek();
}

Card* KlondikeSolitaire::getTableauCard(int pile, int index) {
    if (pile < 0 || pile > 6) return nullptr;
    return tableau[pile].getCardAt(index);
}

int KlondikeSolitaire::getTableauSize(int pile) {
    if (pile < 0 || pile > 6) return 0;
    return tableau[pile].size();
}

bool KlondikeSolitaire::isStockEmpty() {
    return stock.isEmpty();
}

int KlondikeSolitaire::getScore() {
    return score;
}
