#ifndef __SYSTEM__
#define __SYSTEM__

#include "EventNode.h"

class System : public EventNode 
{
	OBJECT(System);
	BASEOBJECT(System);
public:
	System(Context * context);
	virtual ~System();

	// init
	virtual int Initialize() { return 0; };
	// update
	virtual int Update(int ms) {return 0;};
	// shutdown 
	virtual int Shutdown() { return 0; };
};

#endif