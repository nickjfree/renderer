#ifndef __VECTOR_H__
#define __VECTOR_H__


/*
	Vector container, Need a remove method
*/


using namespace std;

#include <utility>
#include <memory.h>

#define VECTOR_INIT_SIZE 32




template <class T>
class Vector {

private: 
	T * Data;
	int ItemSize;
	int Capacity;
	int Resized;
public:
	class Iterator {
		public:
			T* ptr;
			T& operator * () {
				return *ptr;
			}
			Iterator& operator ++(int ) {
				ptr++;
				return *this;
			}
			bool operator == (Iterator& rh) {
				return ptr == rh.ptr;
			}
			bool operator != (Iterator& rh) {
				return ptr != rh.ptr;
			}
	};
private:
	void Append();
	void Clear();
public:
	Vector() : Data(0), ItemSize(0), Capacity(0), Resized(0) {};
	virtual ~Vector() { Clear(); };
	Vector(Vector&);
	int Size() { return ItemSize; };
	int Resize(int NewSize, int Fill=0);
	T& operator [](int Index);
	void operator = (Vector&);
	int PushBack(T& Item);
	T& PopBack();
	void Empty() {
		// you can't call this function with none pointer types
		//assert(sizeof(T) == sizeof(void*));
		ItemSize = 0;
	}
	Iterator Begin() {
		Iterator Iter;
		Iter.ptr = &Data[0];
		return Iter;
	};
	Iterator End() {
		Iterator Iter;
		Iter.ptr = &Data[ItemSize];
		return Iter;
	};
	T * GetData() {
		return Data;
	}
};


template <class T>
int Vector<T>::Resize(int NewSize, int Fill) {
	if (NewSize > Capacity) {
		T * newData = new T[NewSize];
		for (int i = 0; i < ItemSize; i++) {
			newData[i] = std::move(Data[i]);
		}
		//memcpy(newData, Data, sizeof(T)* ItemSize);
		delete[] Data;
		Data = newData;
		Capacity = NewSize;
		Resized++;
	}
	if (Fill) {
		ItemSize = NewSize;
	}
	return NewSize;
}

template <class T>
T& Vector<T>::operator [](int Index) {
	return Data[Index];
}

template <class T>
void Vector<T>::Clear() {
	if (Capacity) {
		delete[] Data;
		Capacity = ItemSize = 0;
		Data = 0;
	}
}

template <class T>
int Vector<T>::PushBack(T& Item) {
	if (ItemSize == Capacity) {
		Append();
	}
	int Index = ItemSize;
	Data[ItemSize++] = Item;
	return Index;
}

template <class T>
T& Vector<T>::PopBack() {
	return Data[--ItemSize];
}

template <class T>
void Vector<T>::Append() {
	if (ItemSize == Capacity) {
		if (!Capacity) {
			Capacity = VECTOR_INIT_SIZE;
		}
 		Resize(Capacity * 2);
	}
}

template <class T>
Vector<T>::Vector(Vector<T>& rh) {
	ItemSize = rh.ItemSize;
	Capacity = rh.Capacity;
	Resized = rh.Resized;
	Data = new T[ItemSize];
	for (int i = 0; i < ItemSize; i++) {
		Data[i] = rh.Data[i];
	}
}

template <class T>
void Vector<T>::operator=(Vector<T>& rh) {
	ItemSize = rh.ItemSize;
	Capacity = rh.Capacity;
	Resized = rh.Resized;
	Data = new T[ItemSize];
	for (int i = 0; i < ItemSize; i++) {
		Data[i] = rh.Data[i];
	}
//	memcpy(Data, rh.Data, sizeof(T)* ItemSize);
}

#endif
