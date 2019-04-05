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
	// printf("%d\n", lua_gettop(vm));
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
    // push clone function first
    lua_getglobal(vm, "clone");
	// load and put template table on top of the stack
    int ret = scriptingsystem->LoadFile(File);
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
	// set upvalue _ENV  , copy data to gameobject's table
    ret = lua_pcall(vm, 2, 0, 0);
	if (ret) {
		printf("eror pcall: %s %d\n", lua_tostring(vm, -1), __LINE__);
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
	lua_getfield(vm, -2, Callback.ToStr());
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
			printf("eror pcall: %s %d\n", lua_tostring(vm, -1), __LINE__);
		}
		// balance the stack
		lua_pop(vm, 3);
	}
	return 0;
}

int Script::OnDestroy(GameObject * GameObj) {
    // this will completelly remove the reference to gameobject in scripting space
    // so scripts can no longer use this gameobjects
    // gameobject maynot be removed immediatly after this call
    // because of how lua's garbage collection works

	if (ObjectId == -1) {
		return 1;
	}
	// get gameobject table
	lua_getglobal(vm, "entities");
	lua_geti(vm, -1, ObjectId);
	// todo:: set obj's metatable to a default special table to disable operations on this object
    // can't do this now because it will ruin the __gc meta method
	//lua_getglobal(vm, "destroyed_mt");
	//lua_setmetatable(vm, -2);
	lua_pop(vm, 1);
    // set entitis[id] to nil
	lua_pushnil(vm);
	lua_seti(vm, -2, ObjectId);
	// balance the stack
	lua_pop(vm, 1);
	// reset objectid
	ObjectId = -1;
	Component::OnDestroy(GameObj);
    
	return 0;
}