#ifndef __LIST_H__
#define __LIST_H__

#include "../Core/Allocater.h"
#include "../Core/Shortcuts.h"

/*
	Link list of pointers
*/

template <class T>
class List
{

public:
	class ListNode {
	public:
		ListNode * Next;
		ListNode * Prev;
		T* Data;
	};
	class Iterator {
	public:
		ListNode * ptr;
		bool operator == (Iterator& rh) {
			return ptr == rh.ptr;
		}
		bool operator != (Iterator& rh) {
			return ptr != rh.ptr;
		}
		Iterator& operator ++(int) {
			ptr=ptr->Next;
			return *this;
		}
		T* operator * () {
			return ptr->Data;
		}
	};
private:
	ListNode Head;
	Allocater<ListNode> Alloc;
public:
	List();
	virtual ~List();
	void Insert(T* Item);
	void Remove(T* Item);
	void Remove(Iterator& Iter) {
		ListNode * Node = Iter.ptr;
		if (Node != &Head) {
			Node->Prev->Next = Node->Next;
			Node->Next->Prev = Node->Prev;
			Alloc.Free(Node);
		}
	}
	Iterator Find(T* Item) {
		ListNode * Node = Head.Next;
		Iterator Iter;
		while (Node != &Head) {
			if (Node->Data == Item) {
				Iter.ptr = Node;
				return Iter;
			}
			Node = Node->Next;
		}
		Iter.ptr = &Head;
		return Iter;
	};
	Iterator Begin() {
		Iterator Iter;
		Iter.ptr = Head.Next;
		return Iter;
	}
	Iterator End() {
		Iterator Iter;
		Iter.ptr = &Head;
		return Iter;
	}
};




template <class T> List<T>::List() {
	Head.Next = Head.Prev = &Head;
	Head.Data = NULL;
}


template <class T> List<T>::~List() {
	// waht to do?  destruct nodes and decref?
}

template <class T> void List<T>::Insert(T * Item) {
	ListNode * Node = (ListNode*)Alloc.Alloc();
	ListNode * h = &Head;
	Node->Data = Item;
	Node->Next = h->Next;
	Node->Prev = h;
	h->Next->Prev = Node;
	h->Next = Node;
}


template <class T> void List<T>::Remove(T* Item) {
	Iterator Iter = Find(Item);
	Remove(Iter);
}



#endif