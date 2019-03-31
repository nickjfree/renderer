#include "Dict.h"
#include "stdio.h"

Dict::Dict()
{
}


Dict::~Dict()
{
}

template <class T>  T& Dict::Get(const String& key) {
	unsigned int StringHash = (unsigned int)key;
	int Index = HashIndex_.First(StringHash);
	while (Items[Index].key != key) {
		Index = HashIndex_.Next(Index);
	}
	if (Index == -1) {
		KeyValue Item = KeyValue();
		return *(T*)&Item.Value;
	}
	return *(T*)&Items[Index].Value;
}

template <class T>  int Dict::Set(const String& key, T& value) {
	unsigned int StringHash = (unsigned int)key;
	int Index = HashIndex_.First(StringHash);
	while (Items[Index].key != key) {
		Index = HashIndex_.Next(Index);
	}
	if (Index != -1) {
		Items[Index].Value.as<T> = value;
		return Index;
	}
	// new key
	KeyValue Item = KeyValue();
	Item.key = key;
	Item.Value.as<T> = value;
	Index = Items.PushBack(Item);
	HashIndex_.Add(StringHash, Index);
	return Index;
}

Variant& Dict::operator[] (const String& key) {
	unsigned int StringHash = (unsigned int)key;
	int Index = HashIndex_.First(StringHash);
	while (Index != -1 && Items[Index].key != key) {
		Index = HashIndex_.Next(Index);
	}
	if (Index != -1) {
		return Items[Index].Value;
	}
	// new key
	KeyValue Item = KeyValue();
	Item.key = key;
	Index = Items.PushBack(Item);
	HashIndex_.Add(StringHash, Index);
	return Items[Index].Value;
}

//Variant& Dict::operator[] (char* key) {
//	unsigned int StringHash = (unsigned int)String(key);
//	int Index = HashIndex_.First(StringHash);
//	while (Index != -1 && Items[Index].key != key) {
//		Index = HashIndex_.Next(Index);
//	}
//	if (Index != -1) {
//		return Items[Index].Value;
//	}
//	// new key
//	KeyValue Item = KeyValue();
//	Item.key = key;
//	Index = Items.PushBack(Item);
//	HashIndex_.Add(StringHash, Index);
//	return Items[Index].Value;
//}
