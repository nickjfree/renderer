#include "ScriptingSystem.h"
#include "ConsoleTask.h"
#include "Tasks\WorkQueue.h"
#include "Core\StringTable.h"


ScriptingSystem::ScriptingSystem(Context * context): System(context) {
}


ScriptingSystem::~ScriptingSystem() {
}


int ScriptingSystem::Update(int ms) {
	return 0;
}

void ScriptingSystem::InitEnvironment() {
	LuaState = luaL_newstate();
	luaL_openlibs(LuaState);
	REGISTER_CLASS(LuaState, GameObject);
}

int ScriptingSystem::Initialize() {
	InitEnvironment();
	// subscribe to level load events
	context->SubscribeFor(this, 400);
	return 0;
}

int ScriptingSystem::HandleEvent(Event *Evt) {
	if (Evt->EventId == 400) {
		Level * level = Evt->EventParam[hash_string::Level].as<Level*>();
		OnLevelLoaded(level);
	}
	return 0;
}

void ScriptingSystem::OnLevelLoaded(Level * level) {
	printf("Level %x loaded\n", level);
	Vector<GameObject *> & GameObjects = level->GetGameObjects();
	Vector<GameObject *>::Iterator Iter;
	for (Iter = GameObjects.Begin(); Iter != GameObjects.End(); Iter++) {
		GameObject * Object = *Iter;
		unsigned int ObjectId = Object->GetObjectId();
		// new table as the object
		lua_newtable(LuaState);
		int exists = luaL_newmetatable(LuaState, "GameObject");
		lua_setmetatable(LuaState, -2);
		// set a userdata to __self field
		void * user_data = lua_newuserdata(LuaState, sizeof(void*));
		*(GameObject **)user_data = Object;
		lua_setfield(LuaState, -2, "__self");
		// set the table as a global test object
		lua_setglobal(LuaState, Object->GetName());
	}
	// load the scripts
	int ret = luaL_loadfile(LuaState, "F:\\proj\\Game11\\Game\\Engine\\Script\\test\\test.lua");
	if (ret) {
		printf("Couldn't load file: %s\n", lua_tostring(LuaState, -1));
	}
	ret = lua_pcall(LuaState, 0, LUA_MULTRET, 0);
	if (ret) {
		printf("eror pcall: %s\n", lua_tostring(LuaState, -1));
	}
	// run debug console
	RunDebugConsole();
	return;
}

void ScriptingSystem::GetConsoleInput() {
	ConsoleTask * task = ConsoleTask::Create();
	memset(DebugBuffer, 0, DEBUG_BUFFER_SIZE);
	task->buffer = DebugBuffer;
	task->Scripting = this;
	WorkQueue * Queue = context->GetSubsystem<WorkQueue>();
	Queue->QueueTask(task);
}

void ScriptingSystem::RunDebugConsole() {
	GetConsoleInput();
}

void ScriptingSystem::RunDebug(char * script) {
	int ret = luaL_loadstring(LuaState, script);
	if (ret) {
		printf("Couldn't load script: %s\n", lua_tostring(LuaState, -1));
	}
	ret = lua_pcall(LuaState, 0, LUA_MULTRET, 0);
	if (ret) {
		printf("error: %s\n", lua_tostring(LuaState, -1));
	}
	// read another line
	GetConsoleInput();
}

