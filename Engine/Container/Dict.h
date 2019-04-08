#ifndef __STATIC_DICT__
#define __STATIC_DICT__

#include "../Core/MathFunc.h"
#include "../Core/Variant.h"
#include "../Core/Str.h"
#include "../Container/Vector.h"
#include "../Container/HashIndex.h"


/*
dict. for game event parameters. hashindex is used to store vector index into hashindex, this is not a hash table
*/

#pragma pack(16)
typedef struct KeyValue {
    String key;
    Variant Value;
}KeyValue;
#pragma pack()


class Dict
{
private:
    Vector<KeyValue> Items;
    HashIndex HashIndex_;
private:
    // get stringhash
    int hash(const char * string) { return djb2_hash((unsigned char*)string); }

public:
    class Iterator {
    public:
        KeyValue *ptr;
        KeyValue& operator * () {
            return *ptr;
        }
        bool operator == (Iterator& rh) {
            return ptr == rh.ptr;
        }
        bool operator != (Iterator& rh) {
            return ptr != rh.ptr;
        }
        Iterator& operator ++(int) {
            ptr++;
            return *this;
        }
    };
public:
    Dict();
    virtual ~Dict();
    template <class T>  T& Get(const String& key);
    template <class T>  int Set(const String& key, T& value);
    void Remove(const String& key);
    Variant& operator[] (const String& key);
    void Clear() { HashIndex_.Reset(); Items.Resize(0, 1); };
    //	Variant& operator[] (char * key);
        // begin, not used
    Iterator Begin() {
        Iterator Iter;
        Iter.ptr = &Items[0];
        return Iter;
    };

    Iterator Find(const String& key) {
        Iterator Iter;
        unsigned int StringHash = (unsigned int)key;
        int Index = HashIndex_.First(StringHash);
        while (Index != -1 && Items[Index].key != key) {
            Index = HashIndex_.Next(Index);
        }
        if (Index == -1) {
            Iter.ptr = &Items[Items.Size()];
        }
        else {
            Iter.ptr = &Items[Index];
        }
        return Iter;
    }

    Iterator End() {
        Iterator Iter;
        Iter.ptr = &Items[Items.Size()];
        return Iter;
    }
};




#endif