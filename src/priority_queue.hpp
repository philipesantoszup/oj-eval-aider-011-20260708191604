#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T data;
        Node *l, *r;
        int npl;

        Node(const T &val) : data(val), l(nullptr), r(nullptr), npl(0) {}
    };

    Node *root;
    size_t _size;
    Compare comp;

    int getNpl(Node *n) const {
        return n ? n->npl : -1;
    }

    void clear(Node *n) {
        if (!n) return;
        clear(n->l);
        clear(n->r);
        delete n;
    }

    Node* copyTree(Node *n) {
        if (!n) return nullptr;
        Node *newNode = new Node(n->data);
        newNode->l = copyTree(n->l);
        newNode->r = copyTree(n->r);
        newNode->npl = n->npl;
        return newNode;
    }

    Node* mergeNodes(Node *a, Node *b) {
        if (!a) return b;
        if (!b) return a;

        // Use the provided Compare. If it throws, the exception propagates.
        // We want a max-heap, so we want the larger element at the root.
        // std::less<T> returns true if a < b.
        if (comp(a->data, b->data)) {
            // b is larger or equal, b becomes root
            std::swap(a, b);
        }

        // a is now the root
        a->r = mergeNodes(a->r, b);

        if (getNpl(a->l) < getNpl(a->r)) {
            std::swap(a->l, a->r);
        }
        a->npl = getNpl(a->r) + 1;
        return a;
    }

public:
	/**
	 * @brief default constructor
	 */
	priority_queue() : root(nullptr), _size(0) {}

	/**
	 * @brief copy constructor
	 * @param other the priority_queue to be copied
	 */
	priority_queue(const priority_queue &other) : root(nullptr), _size(other._size), comp(other.comp) {
		root = copyTree(other.root);
	}

	/**
	 * @brief deconstructor
	 */
	~priority_queue() {
		clear(root);
	}

	/**
	 * @brief Assignment operator
	 * @param other the priority_queue to be assigned from
	 * @return a reference to this priority_queue after assignment
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this != &other) {
			Node *newRoot = copyTree(other.root);
			clear(root);
			root = newRoot;
			_size = other._size;
			comp = other.comp;
		}
		return *this;
	}

	/**
	 * @brief get the top element of the priority queue.
	 * @return a reference of the top element.
	 * @throws container_is_empty if empty() returns true
	 */
	const T & top() const {
		if (empty()) throw container_is_empty();
		return root->data;
	}

	/**
	 * @brief push new element to the priority queue.
	 * @param e the element to be pushed
	 */
	void push(const T &e) {
		Node *newNode = new Node(e);
		Node *oldRoot = root;
		try {
			root = mergeNodes(newNode, root);
			_size++;
		} catch (...) {
			// If mergeNodes throws (due to Compare), we must restore the original state.
			// Since newNode was just created and not yet linked into the original tree,
			// we just delete it and keep the old root.
			delete newNode;
			root = oldRoot;
			throw;
		}
	}

	/**
	 * @brief delete the top element from the priority queue.
	 * @throws container_is_empty if empty() returns true
	 */
	void pop() {
		if (empty()) throw container_is_empty();
		
		Node *oldRoot = root;
		try {
			root = mergeNodes(root->l, root->r);
			_size--;
			delete oldRoot;
		} catch (...) {
			// Restore root to original state
			root = oldRoot;
			throw;
		}
	}

	/**
	 * @brief return the number of elements in the priority queue.
	 * @return the number of elements.
	 */
	size_t size() const {
		return _size;
	}

	/**
	 * @brief check if the container is empty.
	 * @return true if it is empty, false otherwise.
	 */
	bool empty() const {
		return root == nullptr;
	}

	/**
	 * @brief merge another priority_queue into this one.
	 * The other priority_queue will be cleared after merging.
	 * The complexity is at most O(logn).
	 * @param other the priority_queue to be merged.
	 */
	void merge(priority_queue &other) {
		if (other.empty()) return;
		if (this->empty()) {
			this->root = other.root;
			this->_size = other._size;
			other.root = nullptr;
			other._size = 0;
			return;
		}

		Node *thisOldRoot = this->root;
		Node *otherOldRoot = other.root;
		
		try {
			// To ensure strong exception safety, we merge the roots.
			// If mergeNodes throws, this->root and other.root must remain unchanged.
			Node *newRoot = mergeNodes(thisOldRoot, otherOldRoot);
			this->root = newRoot;
			this->_size += other._size;
			other.root = nullptr;
			other._size = 0;
		} catch (...) {
			// Restore original roots
			this->root = thisOldRoot;
			other.root = otherOldRoot;
			throw;
		}
	}
};

}

#endif
