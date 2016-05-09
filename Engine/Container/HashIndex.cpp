#include "HashIndex.h"
#include <memory.h>


HashIndex::HashIndex() {
	HashSize = DEFAULT_HASHSIZE;
	IndexSize = DEFAULT_HASHSIZE;
	HashEntry = new int[HashSize];
	IndexEntry = new int[IndexSize];
	memset(HashEntry, -1, HashSize * sizeof(int));
	memset(IndexEntry, -1, IndexSize * sizeof(int));
	HashMask = HashSize - 1;
}

HashIndex::HashIndex(int HashSize, int IndexSize): HashEntry(0),IndexEntry(0) {
	this->HashSize = HashSize;
	this->IndexSize = IndexSize;
	HashEntry = new int[HashSize];
	IndexEntry = new int[IndexSize];
	HashMask = HashSize - 1;
}

HashIndex::~HashIndex() {
	if (HashEntry) {
		delete HashEntry;
	}
	if (IndexEntry) {
		delete IndexEntry;
	}
}

int HashIndex::Resize(int HashSize, int IndexSize) {
	// do not resize hash for now, i don't know how
	if (IndexSize > this->IndexSize) {
		int * newIndex = new int[IndexSize];
		memcpy(newIndex, IndexEntry, sizeof(int)* this->IndexSize);
		memset(newIndex + IndexSize, -1, IndexSize - this->IndexSize);
		IndexEntry = newIndex;
		this->IndexSize = IndexSize;
	}
	return 0;
}

int HashIndex::Add(int key, int index) {
	int hash_index = key & HashMask;
	if (index > IndexSize) {
		Resize(0, (index+1) * 2);
	}
	IndexEntry[index] = HashEntry[hash_index];
	HashEntry[hash_index] = index;
	return -1;
}

int HashIndex::First(int key) {
	int hash_index = key & HashMask;
	return HashEntry[hash_index];
}

int HashIndex::Next(int index) {
	return IndexEntry[index];
}

int HashIndex::Remove(int key, int index) {
	int hash_index = key & HashMask;
	int Index = First(key);
	int Prev = Index;
	if (Index == index) {
		HashEntry[hash_index] = IndexEntry[Index];
	}
	else {
		while (Index != index) {
			Prev = Index;
			Index = Next(Index);
		}
		IndexEntry[Prev] = IndexEntry[Index];
	}
	IndexEntry[Index] = -1;
	return -1;
}

int HashIndex::Reset() {
	memset(HashEntry, -1, HashSize * sizeof(int));
	memset(IndexEntry, -1, IndexSize * sizeof(int));
	return -1;
}
