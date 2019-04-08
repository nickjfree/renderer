#ifndef __LINEAR_BUFFER__
#define __LINEAR_BUFFER__

#include "Vector.h"
/*
    linear buffer
*/

template <class T, int Size = 8192>
class LinearBuffer
{
private:
    T Data[Size];
    Vector<int> Free;
    int MaxIndex;
    CRITICAL_SECTION Lock;
public:
    LinearBuffer() { MaxIndex = 0; InitializeCriticalSection(&Lock); };
    ~LinearBuffer() {};
    int AddItem(T& data) {
        EnterCriticalSection(&Lock);
        int Index = MaxIndex;
        if (Free.Size() > 0) {
            Index = Free.PopBack();
        }
        else {
            Index = MaxIndex++;
        }
        Data[Index] = data;
        LeaveCriticalSection(&Lock);
        return Index;
    }

    int MarkFree(int Id) {
        EnterCriticalSection(&Lock);
        Free.PushBack(Id);
        LeaveCriticalSection(&Lock);
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