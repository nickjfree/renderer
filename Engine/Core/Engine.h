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
	// handle message
	int OnMessage(int msg, size_t lParam, size_t wParam);
	// Shutdown
	int Shutdown();
};


#endif
