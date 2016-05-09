#ifndef __HASH_MAP__
#define __HASH_MAP__


#include "../Core/Allocater.h"
/*
  HashMap
*/



template <class K, class V, int HashSize=32>
class HashMap {

public:
	class KeyValue {
		public:
			KeyValue() {};
			~KeyValue() {};
			unsigned int  Key;
			KeyValue * Next, * Prev;
			V Value;
	};

	class Iterator {
	public:
		KeyValue * ptr;
		V& operator * () {
			return ptr->Value;
		}
		bool operator == (Iterator& rh) {
			return ptr == rh.ptr;
		}
		bool operator != (Iterator& rh) {
			return ptr != rh.ptr;
		}
	};
private:
	Allocater<KeyValue> Alloc;
	KeyValue Entry[HashSize];
	int Mask;
	int Size;

private:
	void Insert(int Index, KeyValue* kv) {
		KeyValue * head = &Entry[Index];
		kv->Prev = head;
		kv->Next = head->Next;
		head->Next->Prev = kv;
		head->Next = kv;
	}

	void Remove(KeyValue* kv) {
		kv->Prev->Next = kv->Next;
		kv->Next->Prev = kv->Prev;
	}

public:
	HashMap() {
		for (int i = 0; i < HashSize; i++) {
			Entry[i].Next = Entry[i].Prev = &Entry[i];
			Entry[i].Key = -1;
		}
		Mask = HashSize - 1;
	}
	virtual ~HashMap() {

	}

	// get
	V& Get(K& k) {
		int Key = (int)k;
		int Index = Key & Mask;
		KeyValue * head= &Entry[Index];
		KeyValue * kv = head->Next;
		while (kv != head && kv->Key != Key) {
			kv = kv->Next;
		}
		if (kv == head) {
			kv = (KeyValue*)Alloc.Alloc();
			kv = new (kv) KeyValue;
			kv->Key = Key;
			Insert(Index, kv);
		}
		return kv->Value;
	}
	Iterator Find(K& k) {
		int Key = (int)k;
		unsigned int Index = Key & Mask;
		KeyValue * head = &Entry[Index];
		KeyValue * kv = head->Next;
		while (kv != head && kv->Key != Key) {
			kv = kv->Next;
		}
		Iterator Iter;
		if (kv == head) {
			Iter.ptr = NULL;
		}
		else {
			Iter.ptr = kv;
		}
		return Iter;
	}
	// set
	void Set(K& k, V& v) {
		Get(k) = v;
	}

	// remove
	void Erase(K& k) {
		Iterator Iter = Find(k);
		if (Iter != End()) {
			Remove(Iter.ptr);
		}
 	}
	
	// operator []
	V& operator [] (K& k) {
		return Get(k);
	}

	// begin, not used
	Iterator Begin() {
		Iterator Iter;
		Iter.ptr = -1;
		return Iter;
	};

	Iterator End() {
		Iterator Iter;
		Iter.ptr = NULL;
		return Iter;
	}
};


#endif
