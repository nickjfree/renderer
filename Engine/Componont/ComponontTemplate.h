#include "Componont.h"
#include "ObjectContainer.h"

template <class T>
class ComponontTemplate : public Componont
{
private:
	static CObjectContainer m_Container;

public:
	ComponontTemplate(void);
	void * operator new(size_t size);
};

template<class T>
CObjectContainer ComponontTemplate<T>::m_Container;


template <class T>
ComponontTemplate<T>::ComponontTemplate(void)
{
}

template <class T>
void * ComponontTemplate<T>::operator new(size_t size)
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







