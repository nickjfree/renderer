#ifndef __RES_CONTAINER__
#define __RES_CONTAINER__

#include "ObjectContainer.h"
#include <new>

namespace ResourceManager{

template <class T>
class CResourceContainer
{
private:
	CObjectContainer m_Container;
public:
	CResourceContainer(void);
	int AllocResource(T** pResource);
	int FreeResource(int ID);
	int Destroy(void);
	T * GetResourceByID(int ID);
	T * GetNextResource(T * Resource);
};


template<typename T>
ResourceManager::CResourceContainer<T>::CResourceContainer(void)
{
	m_Container.Init(sizeof(T));
}

template<typename T>
int ResourceManager::CResourceContainer<T>::AllocResource(T** pResource)
{
	int ID = m_Container.AllocOne((void**)pResource);
	*pResource = new (*pResource) T;
	return ID;
}

template<typename T>
int ResourceManager::CResourceContainer<T>::FreeResource(int ID)
{
	T * Resource = (T*)m_Container.GetObjectByID(ID);
	Resource->~T();                                       // destroy
	m_Container.ReleaseOne(ID);
	return 0;
}

template<typename T>
int ResourceManager::CResourceContainer<T>::Destroy(void)
{
	m_Container.Destroy();
	return 0;
}

template <typename T>
T * ResourceManager::CResourceContainer<T>::GetResourceByID(int ID)
{
	T * Resource;
	if(ID == -1)
		return NULL;
	Resource = (T*)m_Container.GetObjectByID(ID);
	return Resource;
}

template <typename T>
T * CResourceContainer<T>::GetNextResource(T * Resource)
{
	return (T*)m_Container.GetNext(Resource);
}

}

#endif