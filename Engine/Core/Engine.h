#ifndef __ENGINE__
#define __ENGINE__

#include "Object.h"

/*
	The Engine, managed all the system and update sequence
*/


class Engine
{
private:

protected:
	// context
	Context * context;
public:
	Engine();
	virtual ~Engine();
	// initialize core engine
	int Initialize(void);
	// initalize subsystems
	virtual int InitSubsystems();
	// Update
	int Update(int ms);
	// Shutdown
	int Shutdown();
};


#endif
