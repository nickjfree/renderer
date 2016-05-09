#ifndef __COMPONENT__
#define __COMPONENT__


#include "../Core/Allocater.h"
#include "../Core/EventHandler.h"
#include "../Core/Shortcuts.h"
#include "../Core/Allocater.h"

// basic componont class
class Component : public EventHandler
{
public:
	Component();
	virtual ~Component();
	DECLAR_ALLOCATER(Component);
};

#endif
