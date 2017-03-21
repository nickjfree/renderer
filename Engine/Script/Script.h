#ifndef __SCRIPT_H__
#define __SCRIPT_H__
#include "Scene\Component.h"
#include "Scene\GameObject.h"
#include "Script\LuaStack.h"
#include "Script\Proxy.h"


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


class Script : public Component {

private:
	// the script path
	String File;
	// the id in global table
	unsigned int Id;
	// the luastat
	lua_State * vm;
	// initialized
	bool Initialized;
public:
	Script(Context * Context_);
	virtual ~Script();

	// update
	int Update(int ms);

};

#endif

