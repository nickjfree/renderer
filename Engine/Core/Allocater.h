
#ifndef __Allocable__
#define __Allocable__

#include "Core\ObjectContainer.h"
#include <new>

template <class T, int align = 16>
class Allocater
{
private:
	static ObjectContainer* m_ObjectContainer;
	int Align = align - sizeof(int);
public:
	Allocater() {};
	~Allocater() {};
	void* Alloc();
	void   Free(void* ptr);
	int GetId(void* ptr);
	int Count();
};

template <class T, int align>
ObjectContainer* Allocater<T, align>::m_ObjectContainer = new ObjectContainer(sizeof(T) + sizeof(int) + align - sizeof(int));


template <class T, int align>
void* Allocater<T, align>::Alloc()
{

	void* ptr;
	int id = m_ObjectContainer->AllocOne(&ptr);
	unsigned long long offset = (unsigned long long)(Align - (unsigned long long)ptr) & 0x0F;
	ptr = (void*)((unsigned long long)ptr + offset);
	*(int*)ptr = id;
	return (char*)ptr + sizeof(int);
}

template <class T, int align>
int Allocater<T, align>::GetId(void* ptr)
{
	int id = *(int*)((char*)ptr - sizeof(int));
	return id;
}

template <class T, int align>
void Allocater<T, align>::Free(void* ptr)
{
	int id = *(int*)((char*)ptr - sizeof(int));
	m_ObjectContainer->ReleaseOne(id);
}

template <class T, int align>
int Allocater<T, align>::Count()
{
	return m_ObjectContainer->Count();
}

#endif
