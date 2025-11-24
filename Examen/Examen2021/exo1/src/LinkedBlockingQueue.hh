#pragma once
#include <string> // size_t
#include <mutex>
#include <atomic>
#include <condition_variable>


namespace pr {
/**
 * An optionally-bounded queue based on
 * linked nodes.
 * This queue orders elements FIFO (first-in-first-out).
 * The <em>head</em> of the queue is that element that has been on the
 * queue the longest time.
 * The <em>tail</em> of the queue is that element that has been on the
 * queue the shortest time. New elements
 * are inserted at the tail of the queue, and the queue retrieval
 * operations obtain elements at the head of the queue.
 *  Linked nodes are
 * dynamically created upon each insertion unless this would bring the
 * queue above capacity.
 *
 * @param <E> the type of elements held in this queue
 */
template<typename E>
class LinkedBlockingQueue {
    /**
     * Linked list node class/struct
     */
     struct Node {
    	/** always nullptr for the head of the list */
        E * item;

        /**
         * One of:
         * - the real successor Node
         * - nullptr, meaning there is no successor (this is the last node)
         */
        Node * next;

        Node(E * x):item(x),next(nullptr) {}
    };

    /** The capacity bound, or Integer.MAX_VALUE if none */
    const size_t capacity;
    /** Current number of elements */
    std::atomic<size_t> count;
    /**
     * Head of linked list.
     * Invariant: head->item == nullptr
     */
    Node *head;
    /**
     * Tail of linked list.
     * Invariant: last->next == nullptr
     */
    Node *last;

    std::mutex mt;
    std::mutex mtEmpty;
    std::mutex mtFull;
    std::condition_variable notEmpty;
    std::condition_variable notFull;

    /**
     * Links node at end of queue.
     *     Enqueue doit correctement mettre à jour la liste pour insérer en fin de liste le noeud argument.
     *
     * @param node the node
     */
    void enqueue(Node * node) {
        // assert last->next == nullptr;
        if(!head->next)
            head->next = node;
        else 
            last->next = node;
        last = node;
    }

    /**
     * Removes a node from head of queue.
     *     Dequeue doit lire et décaler la tête de la liste, et ne pas oublier de libérer la mémoire du chaînon lu.
     * @return the node
     */
     E * dequeue() {
        // assert head->item == nullptr;
        Node * tete = head->next;
        head->next = tete->next;
        tete->next = nullptr;
        E * elm = tete->item;
        delete tete;
        return elm;
    }

public :

    /*
    Notons qu'au lieu d'avoir head == nullptr quand la liste est vide (comme on a fait en TD), le choix
    fait ici est de toujours avoir un chainon de tête dont l'item vaut nullptr (head− > item == nullptr
    est un invariant de la classe), et un chaînon de queue dont le next vaut nullptr (last− > next ==
    nullptr est un invariant de la classe). Quand la queue est vide head == last.
    */
    
    /**
     * Creates a queue with the given (fixed) capacity.
     * @param capacity the capacity of this queue
     */
     LinkedBlockingQueue(size_t capacity) :  capacity(capacity), count(0), head(new Node(nullptr)), last(head){
      //  if (capacity <= 0) throw "IllegalArgumentException()";
    }

    ~LinkedBlockingQueue() {
        Node * succ;
        for(Node * curr = head; curr; curr = succ) {
            succ = curr->next;
            delete curr;
        }
    }
    /**
     * Returns the number of elements in this queue.
     */
    size_t size() const {
        std::unique_lock<std::mutex> lk(mt);
        return count;
    }

    /**
     * Returns the number of additional elements that this queue can ideally
     * (in the absence of memory or resource constraints) accept without
     * blocking. This is always equal to the initial capacity of this queue
     * less the current size of this queue.
     */
    size_t remainingCapacity() const {
        std::unique_lock<std::mutex> lk(mt);
        return capacity - count;
    }

    /**
     * Inserts the specified element at the tail of this queue
     *
     * // TODO Question 2
     * or return false if full
     *
     */
    bool put(E * e) {
        // if (e == nullptr) throw "NullPointerInserted()";
        size_t c;
        {
            std::unique_lock<std::mutex> lk(mtFull);
            notFull.wait(lk, [this]{return (capacity - count) > 0;});
            Node * node = new Node(e);
            enqueue(node);
            c = count++;
        }
        if(c <= 0)
            notEmpty.notify_one();
        // si le thread courant ne vient pas de remplir la queue on notifie le prod suivant
        if(capacity  >  (c + 1))
            notFull.notify_one();
        return true;
    }

    /**
     * Takes and returns the element at the head of the queue
     *
     * // TODO Question 2
     * or return nullptr if empty.
     *
     */
    E * take() {
        E * x;
        size_t c;
        {
            std::unique_lock<std::mutex> lk(mtEmpty);
            notEmpty.wait(lk, [this]{return count > 0;});
            x = dequeue();
            c = count--;
        }
        if(capacity - c <= 0)
            notFull.notify_one();
        // si le thread courant ne vient pas de vider la queue on notifie le cons suivant
        if(c - 1 > 0)
            notEmpty.notify_one();
    	return x;
    }

} ; // end class

} // namespace
