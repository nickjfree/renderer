#ifndef __LINEAR_BUFFER__
#define __LINEAR_BUFFER__

#include "Vector.h"
/*
	linear buffer
*/

template <class T, int Size=8192>
class LinearBuffer
{
private:
	T Data[Size];
	Vector<int> Free;
	int MaxIndex;
public:
	LinearBuffer() { MaxIndex = 0; };
	~LinearBuffer(){};
	int AddItem(T& data) {
		int Index = MaxIndex;
		if (Free.Size() > 0) {
			Index = Free.PopBack();
		}
		else {
			Index = MaxIndex++;
		}
		Data[Index] = data;
		return Index;
	}

	int MarkFree(int Id) {
		Free.PushBack(Id);
		return Id;
	}

	T& GetItem(int Id) {
		return Data[Id];
	}

	T& operator [](int Id) {
		return Data[Id];
	}
};

#endif