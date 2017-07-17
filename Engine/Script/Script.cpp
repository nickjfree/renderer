#include "Script.h"
#include "ScriptingSystem.h"



USING_ALLOCATER(Script);

Script::Script(Context * Context_): Component(Context_) {
	scriptingsystem = Context_->GetSubsystem<ScriptingSystem>();
	vm = scriptingsystem->GetVM();
}

Script::~Script() {
	printf("destroyed\n");
}

void Script::Start() {
	// run on_start in gameobject's _ENV
	if (ObjectId == -1) {
		return;
	}
	lua_getglobal(vm, "objects");
	lua_geti(vm, -1, ObjectId);
	lua_getfield(vm, -1, "on_start");
	// push ms
	lua_pushvalue(vm, -2);
	lua_setupvalue(vm, -2, 1);
	int ret = lua_pcall(vm, 0, LUA_MULTRET, 0);
	if (ret) {
		printf("eror pcall: %s\n", lua_tostring(vm, -1));
	}
	lua_pop(vm, 2);
}

int Script::Update(int ms) {
	// run update in gameobject's _ENV
	if (ObjectId == -1) {
		return 1;
	}
	lua_getglobal(vm, "objects");
	lua_geti(vm, -1, ObjectId);
	lua_getfield(vm, -1, "update");
	// push ms
	lua_pushinteger(vm, ms);
	lua_pushvalue(vm, -3);
	lua_setupvalue(vm, -3, 1);
	int ret = lua_pcall(vm, 1, LUA_MULTRET, 0);
	if (ret) {
		printf("eror pcall: %s\n", lua_tostring(vm, -1));
	}
	lua_pop(vm, 2);
	return 0;
}

int Script::OnAttach(GameObject * GameObj) {
	LuaStack::Export(vm, GameObj);
	ObjectId = GameObj->GetObjectId();
 	Register();
	Start();
	Initialized = true;
	return 0;
}

// register 
void Script::Register() {
	//File = "F:\\proj\\Game11\\Game\\Engine\\Script\\test\\script.lua";
	int ret = luaL_loadfile(vm, File);
	if (ret) {
		printf("Couldn't load script: %s\n", lua_tostring(vm, -1));
	}
	// get gameobject _ENV for sandboxing
	lua_getglobal(vm, "objects");
	lua_geti(vm, -1, ObjectId);
	lua_remove(vm, -2);
	// set _ENV
	lua_pushvalue(vm, -1);
	lua_setfield(vm, -2, "gameobject");
	// set global env to "engine"
	lua_pushglobaltable(vm);
	lua_setfield(vm, -2, "engine");
	// set event table
	lua_newtable(vm);
	lua_setfield(vm, -2, "event");
	// set upvalue _ENV to function
	lua_setupvalue(vm, -2, 1);
	// run file
	lua_pcall(vm, 0, LUA_MULTRET, 0);
	if (ret) {
		printf("eror pcall: %s\n", lua_tostring(vm, -1));
	}
	scriptingsystem->RegisterScript(this);
}
// remove script
void Script::Remove() {
	scriptingsystem->RemoveScript(this);
}

int Script::Subscribe(int Event, String& Callback) {
	if (ObjectId == -1) {
		return 1;
	}
	Component::Subscribe(Event, Callback);
	// set event to function mapping in gameobject's event_table
	// get gameobhect
	lua_getglobal(vm, "objects");
	lua_geti(vm, -1, ObjectId);
	lua_getfield(vm, -1, "event");
	// get callback functions
	lua_getfield(vm, -2, Callback);
	lua_seti(vm, -2, Event);
	// done. clear the stack
	lua_pop(vm, 3);
	return 0;
}

//handle event
int Script::HandleEvent(Event * Evt) {
	if (ObjectId == -1) {
		return 1;
	}
	// user event is larger than USER_EVENT
	GameObject * object = this->Owner;
	if (Evt->EventId > USER_EVENT) {
		// this is event sent from scripts	
		// get evnet table
		lua_getglobal(vm, "objects");
		lua_geti(vm, -1, ObjectId);
		lua_getfield(vm, -1, "event");
		// call callback within _ENV
		lua_geti(vm, -1, Evt->EventId);
		lua_pushvalue(vm, -3);
		lua_setupvalue(vm, -2, 1);
		int ret = lua_pcall(vm, 0, LUA_MULTRET, 0);
		if (ret) {
			printf("eror pcall: %s\n", lua_tostring(vm, -1));
		}
		// balance the stack
		lua_pop(vm, 3);
	}
	return 0;
}

int Script::OnDestroy(GameObject * GameObj) {
	if (ObjectId == -1) {
		return 1;
	}
	// get gameobject table
	lua_getglobal(vm, "entities");
	lua_geti(vm, -1, ObjectId);
	// set obj's metatable to a default special table to disable operations on this object
	lua_getglobal(vm, "destroyed_mt");
	lua_setmetatable(vm, -2);
	lua_pop(vm, 1);
	lua_pushnil(vm);
	lua_seti(vm, -2, ObjectId);
	// balance the stack
	lua_pop(vm, 1);
	// reset objectid
	ObjectId = -1;
	// remove from script list
	Remove();
	// disable event
	DisableEvent();
	return 0;
}