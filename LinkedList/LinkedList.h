// Created by Yehor on 10.02.2021.

#ifndef UNIVPROJECT_LINKEDLIST_H
#define UNIVPROJECT_LINKEDLIST_H

#include <string>

using namespace std;

class LinkedList {
private:
    class Node {
    public:
        int value;
        Node* prev;
        Node* next;
    public:
        Node(int value, Node* prev, Node* next);
    };

    Node* head;
    Node* tail;
    int size;   // size of the list

    /**
     * Finds a node at position {@code index}
     * @param index - a position where a node is places
     * @return found node
     */
    Node* getNodeAt(int index);

    /**
     * Deletes all the elements of the list
     */
    void deleteList();

    // destructor
    ~LinkedList();
public:
    // basic constructor
    LinkedList();

    /**
     * @return a number of element in the list
     */
    int getSize();

    /**
     * @param index - index of element which value will be got
     * @return - value at the specified index
     */
    int get(int index);

    /**
     * This operator returns the same result as the method {@code get(int index)}
     * @param index - index of element which value will be got
     * @return - value at the specified index
     */
    int operator [](int index);

    /**
     * Adds an element at the specified index
     * @param index - index where an element will be places
     * @param value - a value of the new element
     */
    void add(int index, int value);

    /**
     * Adds an element as the first element.
     * This method is the same as {@code add(index: 0, value: ...).
     * @param value - a value of the new element
     */
    void addFirst(int value);

    /**
     * Adds an element as the last element.
     * This method is the same as {@code add(index: size - 1, value: ...).
     * @param value - a value of the new element
     */
    void addLast(int value);

    /**
     * Changes a value of the element at position {@code index}
     * @param index - an index of element which value will be changed
     * @param newValue - a new value which will be set
     */
    void set(int index, int newValue);

    /**
     * Remove an element at the specified index
     * @param index - index at which an element will be removed
     * @return a value of the deleted element
     */
    int remove(int index);

    /**
     * Remove an element at the first position.
     * This method is the same as {@code remove(index: 0).
     * @return a value of the deleted element
     */
    int removeFirst();

    /**
     * Remove an element at the last position.
     * This method is the same as {@code remove(index: size - 1).
     * @return a value of the deleted element
     */
    int removeLast();

    /**
     * @param value - a value which will be used to check if the element is present in the list
     * @return {@code true} if the element is present. Otherwise, {@code false}
     */
    bool contains(int value);

    /**
     * @return a string representation of the list
     */
    string toString();
};

#endif //UNIVPROJECT_LINKEDLIST_H
