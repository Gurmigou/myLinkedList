// Created by Yehor on 10.02.2021.

#include "LinkedList.h"

LinkedList::Node::Node(int value, LinkedList::Node* prev, LinkedList::Node* next) {
    this->value = value;
    this->prev = prev;
    this->next = next;
}

LinkedList::LinkedList() {
    this->head = nullptr;
    this->tail = nullptr;
    this->size = 0;
}

LinkedList::Node* LinkedList::getNodeAt(int index) {
    Node* cur;
    if (index > size / 2) {
       // start searching for an element from the end
        cur = this->tail;
        for (int i = 0; i < size - index - 1; ++i)
            cur = cur->prev;
    } else {
        // start searching for an element from the begin
        cur = this->head;
        for (int i = 0; i < index; ++i)
            cur = cur->next;
    }
    return cur;
}

void LinkedList::deleteList() {
    if (size != 0) {
        Node* cur = this->head;
        Node* toDelete;
        while (cur != nullptr) {
            toDelete = cur;
            cur = cur->next;
            delete toDelete;
        }
    }
}

LinkedList::~LinkedList() {
    deleteList();
}

int LinkedList::getSize() {
    return size;
}

void LinkedList::add(int index, int value) {
    if (index == 0)
        addFirst(value);
    else if (index == size)
        addLast(value);
    else if (index < 0 || index >= size)
        throw exception();
    else if (size == 0)
        throw exception();
    else {
        // [] - [] - [] - [search_for_this] - insert here - []
        Node* prevBeforeNew = getNodeAt(index - 1);
        Node* insert = new Node(value, prevBeforeNew, prevBeforeNew->next);
        prevBeforeNew->next = insert;
        insert->next->prev = insert;
        this->size++;
    }
}

void LinkedList::addFirst(int value) {
    Node* newHead = new Node(value, nullptr, this->head);
    if (this->head != nullptr)
        this->head->prev = newHead;
    else
        this->tail = newHead;
    this->head = newHead;
    this->size++;
}

void LinkedList::addLast(int value) {
    Node* newTail = new Node(value, this->tail, nullptr);
    if (this->tail != nullptr)
        this->tail->next = newTail;
    else
        this->head = newTail;
    this->tail = newTail;
    this->size++;
}

void LinkedList::set(int index, int newValue) {
    if (index < 0 || index >= size)
        throw exception();
    if (size == 0)
        return;
    Node* required = getNodeAt(index);
    required->value = newValue;
}

int LinkedList::get(int index) {
    if (index < 0 || index >= size || size == 0)
        throw exception();
    return getNodeAt(index)->value;
}

int LinkedList::operator[](int index) {
    return get(index);
}

bool LinkedList::contains(int value) {
    Node* cur = this->head;
    for (int i = 0; i < size; ++i) {
        if (cur->value == value)
            return true;
        cur = cur->next;
    }
    return false;
}

string LinkedList::toString() {
    string result = "";
    Node* cur = this->head;
    for (int i = 0; i < size; ++i) {
        result.append("[").append(to_string(cur->value)).append("]");
        cur = cur->next;
    }
    return result;
}

int LinkedList::remove(int index) {
    if (index < 0 || index >= size || size == 0)
        throw exception();
    if (index == 0)
        return removeFirst();
    else if (index == size - 1)
        return removeLast();
    else {
        Node* toDeleteNode = getNodeAt(index);
        toDeleteNode->prev->next = toDeleteNode->next;
        toDeleteNode->next->prev = toDeleteNode->prev;
        int valueReturn = toDeleteNode->value;
        delete toDeleteNode;
        this->size--;
        return valueReturn;
    }
}

int LinkedList::removeFirst() {
    if (size == 0)
        throw exception();
    Node* prevHead = this->head;
    this->head = this->head->next;
    // having a new head now
    this->head->prev = nullptr;
    int valueReturn = prevHead->value;
    delete prevHead;
    this->size--;
    return valueReturn;
}

int LinkedList::removeLast() {
    if (size == 0)
        throw exception();
    Node* prevTail = this->tail;
    this->tail = this->tail->prev;
    // having a new tail now
    this->tail->next = nullptr;
    int valueReturn = prevTail->value;
    delete prevTail;
    this->size--;
    return valueReturn;
}