// Created by Yehor on 11.02.2021.
#include <string>

using namespace std;

template<class T>
class MyLinkedList {
public:
    class Node {
    public:
        T value;
        Node* prev;
        Node* next;
        Node (T value, Node* prev, Node* next) {
            this->value = value;
            this->prev = prev;
            this->next = next;
        }
        T getValue() const {
            return value;
        }

        Node* getPrev() const {
            return prev;
        }

        Node* getNext() const {
            return next;
        }
    };

private:
    Node* head;
    Node* tail;
    int size;   // size of the list

    /**
     * Finds a node at position {@code index}
     * @param index - a position where a node is places
     * @return found node
     */
    Node* getNodeAt(int index) {
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

    /**
     * Deletes all the elements of the list
     */
    void deleteList() {
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
    // destructor
    ~MyLinkedList() {
        deleteList();
    }
public:
    // basic constructor
    MyLinkedList() {
        this->head = nullptr;
        this->tail = nullptr;
        this->size = 0;
    };

    Node* getHead() {
        return this->head;
    }

    Node* getTail() {
        return this->tail;
    }

    /**
     * @return a number of element in the list
     */
    int getSize() {
        return size;
    }

    /**
     * @param index - index of element which value will be got
     * @return - value at the specified index
     */
    T& get(int index) {
        if (index < 0 || index >= size || size == 0)
            throw exception();
        return getNodeAt(index)->value;
    }

    /**
     * @return head value
     */
    T& getFirst() {
        return this->head->value;
    }

    /**
     * @return tail value
     */
    T& getLast() {
        return this->tail->value;
    }

    /**
     * This operator returns the same result as the method {@code get(int index)}
     * @param index - index of element which value will be got
     * @return - value at the specified index
     */
    T& operator [](int index) {
        return get(index);
    }

    /**
     * Adds an element at the specified index
     * @param index - index where an element will be places
     * @param value - a value of the new element
     */
    void add(int index, T value) {
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

    /**
     * Adds an element as the first element.
     * This method is the same as {@code add(index: 0, value: ...).
     * @param value - a value of the new element
     */
    void addFirst(T value) {
        Node* newHead = new Node(value, nullptr, this->head);
        if (this->head != nullptr)
            this->head->prev = newHead;
        else
            this->tail = newHead;
        this->head = newHead;
        this->size++;
    }

    /**
     * Adds an element as the last element.
     * This method is the same as {@code add(index: size - 1, value: ...).
     * @param value - a value of the new element
     */
    void addLast(T value) {
        Node* newTail = new Node(value, this->tail, nullptr);
        if (this->tail != nullptr)
            this->tail->next = newTail;
        else
            this->head = newTail;
        this->tail = newTail;
        this->size++;
    }

    /**
     * Changes a value of the element at position {@code index}
     * @param index - an index of element which value will be changed
     * @param newValue - a new value which will be set
     */
    void set(int index, T newValue) {
        if (index < 0 || index >= size)
            throw exception();
        if (size == 0)
            return;
        Node* required = getNodeAt(index);
        required->value = newValue;
    }

    /**
     * Remove an element at the specified index
     * @param index - index at which an element will be removed
     * @return a value of the deleted element
     */
    T remove(int index) {
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
            T valueReturn = toDeleteNode->value;
            delete toDeleteNode;
            this->size--;
            return valueReturn;
        }
    }

    /**
     * Remove an element at the first position.
     * This method is the same as {@code remove(index: 0).
     * @return a value of the deleted element
     */
    T removeFirst() {
        if (size == 0)
            throw exception();
        Node* prevHead = this->head;
        this->head = this->head->next;
        // having a new head now
        if (size == 1) {
            this->head = nullptr;
            this->tail = nullptr;
        } else {
            this->head->prev = nullptr;
        }
        T valueReturn = prevHead->value;
        delete prevHead;
        this->size--;
        return valueReturn;
    }

    /**
     * Remove an element at the last position.
     * This method is the same as {@code remove(index: size - 1).
     * @return a value of the deleted element
     */
    T removeLast() {
        if (size == 0)
            throw exception();
        Node* prevTail = this->tail;
        this->tail = this->tail->prev;
        // having a new tail now
        if (size == 1) {
            this->tail = nullptr;
            this->head = nullptr;
        } else {
            this->tail->next = nullptr;
        }
        T valueReturn = prevTail->value;
        delete prevTail;
        this->size--;
        return valueReturn;
    }

    /**
     * @param value - a value which will be used to check if the element is present in the list
     * @return {@code true} if the element is present. Otherwise, {@code false}
     */
    bool contains(T& value) {
        Node* cur = this->head;
        for (int i = 0; i < size; ++i) {
            if (cur->value == value)
                return true;
            cur = cur->next;
        }
        return false;
    }
};