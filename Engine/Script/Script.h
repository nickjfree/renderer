#ifndef __SCRIPT_H__
#define __SCRIPT_H__
#include "Scene\Component.h"
#include "Scene\GameObject.h"
#include "Script\LuaStack.h"
#include "Script\Proxy.h"
#include "ScriptingSystem.h"
#include "Core\Event.h"
#include "Core\Str.h"
#include "Container\HashMap.h"

/*
	Lua scriptfile
*/

/*
	The lua script component is simply a container.
	1. the path of the acture lua script file.
	2. a table contains some key functions such as "Update"  "Init" or even "On<EventName>"
		which is stored in a global table with incremental int as keys
	3. And that table is the acture GameObject exposed to the lua side.
	4. All the key function's first upvalue _ENV is set to be the GameObject table before being called
	5. the table itself is expored as name "self"
*/

class ScriptingSystem;

class Script : public Component {
	OBJECT(Script);
	BASEOBJECT(Script);
	DECLARE_ALLOCATER(Script);

private:
	// the script path
	String File;
	// the id in global table
	unsigned int Id;
	// the luastat
	lua_State* vm;
	// initialized
	bool Initialized;
	// scripting system
	ScriptingSystem* scriptingsystem;
	// subscribed events
	Vector<int> Subscribed;
private:
	// initialize script
	void Start();
	// clenup subscripbed events

public:
	Script(Context* Context_);
	virtual ~Script();
	// handle event
	virtual int HandleEvent(Event* Evt);
	// subscript event
	int Subscribe(int Event, String& Callback);
	// update
	int Update(int ms);
	// test init useing onattach
	virtual int OnAttach(GameObject* GameObj);
	// register 
	void Register();
	// remove script
	void Remove();
	// set script
	void SetScript(const String& file) { File = file; };
	// on destroy
	virtual int OnDestroy(GameObject* GameObj);

};

#endif

