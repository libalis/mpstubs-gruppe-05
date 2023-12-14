/*! \file
 *  \brief Simple \ref Queue (for elements inheriting from \ref Queue::Node)
 */

#pragma once

#include "types.h"
#include "debug/assert.h"

/*! \brief This class implements a simple, singly-linked list of objects
 * implementing the base class \ref Queue::Node.
 *
 * \ref Queue::Node itself merely adds an attribute `_next_node` to the parent
 * object, allowing to enqueue the element into a single \ref Queue.
 *
 * \warning One instance of a class inheriting from \ref Queue::Node can be at
 *          most in one \ref Queue
 *
 * The following example illustrates the usage of \ref Queue::Node :
\verbatim
class Foo : public Queue<Foo>::Node {
  // ...
}
 \endverbatim
 *
 * This Queue implementation supports using C++11 range expressions:
 *
 \verbatim
Queue<Foo> list;
Foo a, b, c;

list.enqueue(&a);
list.enqueue(&b);
list.enqueue(&c);

for(Foo * elem : list) {
  // use elem
}

 \endverbatim
 *
 * \note Implementation details: Unlike in other implementations, the tail
 * pointer does not point to the last element in the queue, but to the last
 * element's next pointer.
 * As long as the queue is empty, tail points to the queue's head pointer;
 * therefore inserting can be implemented without further special cases
 * handling empty queues.
 * The check for emptiness can, however, not be omitted on removal.
 */
template<typename T>
class Queue {
	/* Prevent duplicating any queue, as it would lead to the tail
	 * pointer referring into other queues' data. */

	Queue(const Queue<T>&)            = delete;
	Queue& operator=(const Queue<T>&) = delete;

	T *head;
	T **tail;

 public:
	/*! \brief Default constructor; initialized the queue as empty.
	 */
	Queue() : head(nullptr), tail(&head)  { }

	/*! \brief Enqueues the provided item at the end of the queue.
	 *  \param item  Queue element to be appended.
	 */
	void enqueue(T *item) {
		assert(item != nullptr);
		item->_next_node = nullptr;        // The new node does not have a successor
		*tail = item;               // Append the new node to the end of the queue
		tail = &(item->_next_node);  // Update the tail pointer accordingly
	}

	/*! \brief Removes the first element in the queue and returns it.
	 *  \return The removed head, or `nullptr` if the queue was empty.
	 */
	T* dequeue() {
		T *out = head;
		if (out != nullptr) {  // Check if head is an element in the queue
			head = out->_next_node;
			if (head == nullptr) {  // Was this the last element?
				tail = &head;
			}

			// mark the element as removed
			out->_next_node = nullptr;
		}
		return out;
	}

	/*! \brief A Queue Iterator

	 * This class provides an iterator for our Queue implementation.
	 * The Iterator class encapsulates the state when iterating over the
	 * queue, such as when using C++11 Range Loops.
	 * The Queue Iterators are implemented as [forward iterators](http://en.cppreference.com/w/cpp/concept/ForwardIterator).
	 */
	class Iterator {
		T * first;
	public:
		Iterator() : first(nullptr) {}

		explicit Iterator(Queue<T> * queue) : first(queue->head) {}

		bool operator!=(const Iterator & other) {
			return first != other.first;
		}

		T * operator*() { return first; }

		Iterator & operator++() {
			first = first->_next_node;
			return *this;
		}
	};

	/*! Returns an iterator referring to the head of the queue.
	 */
	Iterator begin() {
		return Iterator(this);
	}

	/*! Returns an end iterator.
	 */
	Iterator end() {
		return Iterator();
	}

	/*! \brief Removes and returns a single element from the queue.
	 *
	 * This method removes and returns a single element (provided
	 * via parameter `item`) from the queue, irrespective of its position.
	 * By default, this function compares pointers; it is possible to
	 * override the default comparator lambda function by specifying a
	 * function type as second parameter.
	 *
	 *  \param item Element to be removed.
	 *  \param cmp  Comparator function.
	 *  \return Returns the removed element, or `0` if no element matches.
	 */
	T* remove(T *item, bool (*cmp)(T*, T*) = [] (T* a, T* b) {return a == b;}) {
		assert(item != nullptr && cmp != nullptr);
		T **prev = &head;
		for (T * current : *this) {
			if (cmp(item, current)) {
				*prev = current->_next_node;
				if (&current->_next_node == tail) {
					tail = prev;
				}
				return current;
			}
			prev = &current->_next_node;
		}
		return nullptr;
	}

	/*! \brief Adds `item` to the beginning of the queue.
	 *  \param item The element to be inserted.
	 */
	void insertFirst(T *item) {
		assert(item != nullptr);
		item->_next_node = head;
		head = item;

		if(tail == &head) {
			tail = &item->_next_node;
		}
	}

	/*! \brief Inserts the element `new_item` directly after `old_item`.
	 *  \param old_item Element to insert after.
	 *  \param new_item Element to be inserted.
	 */
	void insertAfter(T *old_item, T *new_item) {
		assert(old_item != nullptr && new_item != nullptr);
		new_item->_next_node = old_item->_next_node;
		old_item->_next_node = new_item;

		if ((*tail) == new_item) {
			tail = &(new_item->_next_node);
		}
	}

	/*! \brief Returns the first element in the queue without removing it.
	 *  \return The first element in the queue
	 */
	T* first() {
		T *out;
		out = head;
		return out;
	}

	/*! \brief Returns the next element in the queue for a given element.
	 */
	T* next(T* o) {
		assert(o != nullptr);
		return o->_next_node;
	}

	/*! \brief Base class Node for all queueable classes
	 *
	 * Use this class as a base class for every class that should be
	 * queueable. Merely provides a pointer `_next_node` (only accessible from
	 * within the friend class \ref Queue).
	 */
	class Node {
		T* _next_node;
		friend class Queue<T>;

	protected:
		Node() : _next_node(nullptr) {}
	};
};
