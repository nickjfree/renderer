#include "ScriptingSystem.h"
#include "ConsoleTask.h"
#include "Tasks\WorkQueue.h"
#include "Core\StringTable.h"
#include "Scene\GameObject.h"


ScriptingSystem::ScriptingSystem(Context * context): System(context) {
}


ScriptingSystem::~ScriptingSystem() {
}


int ScriptingSystem::Update(int ms) {
	// loop through all script compoment then call update of each compoment
	List<Script>::Iterator Iter;
	for (Iter = Scripts.Begin(); Iter != Scripts.End(); Iter++) {
		Script * script = *Iter;
		script->Update(ms);
	}
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
	// load the initilization scripts
	int ret = luaL_loadfile(LuaState, "F:\\proj\\Game11\\Game\\Engine\\Script\\test\\test.lua");
	if (ret) {
		printf("Couldn't load file: %s\n", lua_tostring(LuaState, -1));
	}
	ret = lua_pcall(LuaState, 0, LUA_MULTRET, 0);
	if (ret) {
		printf("eror pcall: %s\n", lua_tostring(LuaState, -1));
	}
	// export gameobjects
	Vector<GameObject *> & GameObjects = level->GetGameObjects();
	Vector<GameObject *>::Iterator Iter;
	for (Iter = GameObjects.Begin(); Iter != GameObjects.End(); Iter++) {
		GameObject * Object = *Iter;
		LuaStack::Push(LuaState, Object);
		lua_setglobal(LuaState, Object->GetName());
//		lua_pop(LuaState, 1);
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

void ScriptingSystem::RegisterScript(Script * script) {
	Scripts.Insert(script);
}

void ScriptingSystem::RemoveScript(Script * script) {

}

