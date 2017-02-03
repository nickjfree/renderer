#include "Component.h"
#include "ObjectContainer.h"

template <class T>
class ComponentTemplate : public Component
{
private:
	static CObjectContainer m_Container;

public:
	T * next, * prev;
public:
	ComponentTemplate(void);
	void * operator new(size_t size);
};

template<class T>
CObjectContainer ComponentTemplate<T>::m_Container;


template <class T>
ComponentTemplate<T>::ComponentTemplate(void)
{
}

template <class T>
void * ComponentTemplate<T>::operator new(size_t size)
{
	static int first = 1;
	if (first)
	{
		m_Container.Init(size);
		first = 0;
	}
	void * Data = NULL;
	int Id = m_Container.AllocOne(&Data);
	return Data;
}







