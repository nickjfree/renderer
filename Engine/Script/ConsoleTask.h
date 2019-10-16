#ifndef __CONSOLE_TASK__
#define __CONSOLE_TASK__

#include "Tasks\Task.h"
#include "ScriptingSystem.h"

class ConsoleTask : public Task {

	DECLAR_ALLOCATER(ConsoleTask);
	DECLAR_RECYCLE(ConsoleTask);

public:
	// script code line
	char* buffer;
	// system
	ScriptingSystem* Scripting;
public:
	ConsoleTask();
	virtual ~ConsoleTask();
	virtual int Work();
	virtual int Complete();
};

#endif

