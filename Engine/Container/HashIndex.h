#ifndef __HASH_INDEX__
#define __HASH_INDEX__

#include "../Core/MathFunc.h"

/*
 hash from a string to an index, the index then can be used to locate an item in a array
*/
#define DEFAULT_HASHSIZE 32

class HashIndex
{
private:
    int * HashEntry;
    int * IndexEntry;
    int HashSize;
    int IndexSize;
    int HashMask;
public:
    HashIndex();
    HashIndex(int HashSize, int IndexSize);
    int Add(int key, int index);
    int First(int key);
    int Next(int index);
    int Remove(int key, int index);
    int Reset();
    int Resize(int HashSize, int IndexSize);
    virtual ~HashIndex();

};

#endif