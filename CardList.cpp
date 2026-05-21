#include "CardList.h"

CardList::CardList() : head(nullptr), count(0) {}

CardList::~CardList() {
    while (head) {
        Node<Card*>* temp = head;
        head = head->next;
        delete temp;
    }
}

void CardList::addCard(Card* card) {
    Node<Card*>* newNode = new Node<Card*>(card);
    if (!head) {
        head = newNode;
    } else {
        Node<Card*>* temp = head;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    count++;
}

Card* CardList::removeLastCard() {
    if (!head) return nullptr;
    
    if (!head->next) {
        Card* card = head->data;
        delete head;
        head = nullptr;
        count--;
        return card;
    }
    
    Node<Card*>* temp = head;
    while (temp->next->next) {
        temp = temp->next;
    }
    Card* card = temp->next->data;
    delete temp->next;
    temp->next = nullptr;
    count--;
    return card;
}

Card* CardList::getLastCard() const {
    if (!head) return nullptr;
    Node<Card*>* temp = head;
    while (temp->next) {
        temp = temp->next;
    }
    return temp->data;
}

Card* CardList::getCardAt(int index) const {
    Node<Card*>* temp = head;
    for (int i = 0; i < index && temp; i++) {
        temp = temp->next;
    }
    return temp ? temp->data : nullptr;
}

int CardList::size() const { return count; }
bool CardList::isEmpty() const { return head == nullptr; }
Node<Card*>* CardList::getHead() const { return head; }