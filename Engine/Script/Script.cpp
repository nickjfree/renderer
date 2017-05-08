#include "Script.h"
#include "ScriptingSystem.h"



USING_ALLOCATER(Script);

Script::Script(Context * Context_): Component(Context_) {
	scriptingsystem = Context_->GetSubsystem<ScriptingSystem>();
	vm = scriptingsystem->GetVM();
}

Script::~Script() {
}

int Script::Update(int ms) {
	// run update in gameobject's _ENV
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
	Initialized = true;
	return 0;
}

// register 
void Script::Register() {
	File = "F:\\proj\\Game11\\Game\\Engine\\Script\\test\\script.lua";
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

}