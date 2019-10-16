#include "Component.h"
#include "ObjectContainer.h"

template <class T>
class ComponentTemplate : public Component
{
public:
	T* next, * prev;
public:
	ComponentTemplate(void);
};




template <class T>
ComponentTemplate<T>::ComponentTemplate(void)
{
}







