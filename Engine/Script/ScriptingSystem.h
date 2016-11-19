#ifndef __SCRIPTING_SYSTEM__
#define __SCRIPTING_SYSTEM__


#include "Core\System.h"
#include "LuaStack.h"
#include "Proxy.h"
#include "Export.h"
#include "Resource\Level.h"


#define DEBUG_BUFFER_SIZE 1024

class ScriptingSystem : public System {

	BASEOBJECT(System);
	OBJECT(ScriptingSystem);

private:
	lua_State * LuaState;
	char DebugBuffer[DEBUG_BUFFER_SIZE];
private:
	// register classes
	void InitEnvironment();
	// on level loaded
	void OnLevelLoaded(Level * level);
	// run interactive debug console
	void RunDebugConsole();
	// get console input
	void GetConsoleInput();
public:
	ScriptingSystem(Context * context);
	~ScriptingSystem();
	virtual int Update(int ms);

	virtual int Initialize();

	virtual int HandleEvent(Event * Evt);
	// exceute debug 
	void RunDebug(char * script);
};

#endif
