#ifndef CARDLIST_H
#define CARDLIST_H

#include "Card.h"
#include "Node.h"

class CardList {
private:
    Node<Card*>* head;
    int count;
    
public:
    CardList();
    ~CardList();
    
    void addCard(Card* card);
    Card* removeLastCard();
    Card* getLastCard() const;
    Card* getCardAt(int index) const;
    int size() const;
    bool isEmpty() const;
    Node<Card*>* getHead() const;
};

#endif // CARDLIST_H