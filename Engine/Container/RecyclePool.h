#ifndef __RECYCLE_POOL__
#define __RECYCLE_POOL__


#include "Vector.h"
/*
	Recycle Pool, Some nested object is hard to hava a custom allocater. so a recyled pool is good for the job.
	the object in the pool must be reuseable without initaliztion again
*/

template <class T>
class RecyclePool
{
private:
	Vector<T*> Recycled;
public:
	RecyclePool();
	virtual ~RecyclePool();
	T* Create();
	void Recycle(T* Item);
};

template <class T> RecyclePool<T>::RecyclePool() {}

template <class T> RecyclePool<T>::~RecyclePool() {}

template <class T> T* RecyclePool<T>::Create() {
	if (Recycled.Size()) {
		T * Item = Recycled.PopBack();
		return Item;
	}
	else {
		return new T();
	}
}

template <class T> void RecyclePool<T>::Recycle(T* Item) {
	Recycled.PushBack(Item);
}

#endif