#ifndef STACK_H
#define STACK_H

#include "Node.h"

template <typename T>
class Stack {
private:
    Node<T>* top;
    int count;

public:
    Stack() : top(nullptr), count(0) {}

    ~Stack() {
        while (!isEmpty()) {
            pop();
        }
    }

    void push(T data) {
        Node<T>* newNode = new Node<T>(data);
        newNode->next = top;
        top = newNode;
        count++;
    }

    T pop() {
        if (isEmpty()) {
            throw "Stack is empty";
        }
        Node<T>* temp = top;
        T data = top->data;
        top = top->next;
        delete temp;
        count--;
        return data;
    }

    T peek() const {
        if (isEmpty()) {
            throw "Stack is empty";
        }
        return top->data;
    }

    bool isEmpty() const { return top == nullptr; }
    int size() const { return count; }
    Node<T>* getTop() const { return top; }
};

#endif // STACK_H
