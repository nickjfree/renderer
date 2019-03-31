#include "ScriptingSystem.h"
#include "ConsoleTask.h"
#include "Tasks\WorkQueue.h"
#include "Core\StringTable.h"
#include "Scene\GameObject.h"
#include "Scene\Scene.h"
#include "Animation\Animator.h"


ScriptingSystem::ScriptingSystem(Context * context): System(context) {
}


ScriptingSystem::~ScriptingSystem() {
}


int ScriptingSystem::Update(int ms) {
	// loop through all script compoment then call update of each compoment
	List<Script>::Iterator Iter;
	for (Iter = Scripts.Begin(); Iter != Scripts.End(); Iter++) {
		Script * script = *Iter;
		if (!script->Destroyed) {
			script->Update(ms);
		} else {
			Destroyed.PushBack(script);
		}
	}
	// handle destroyed scripts
	int Size = Destroyed.Size();
	for (int i = 0; i < Size; i++) {
		Script * script = Destroyed[i];
		script->Remove();
	}
	Destroyed.Empty();
	return 0;
}

void ScriptingSystem::InitEnvironment() {
	LuaState = luaL_newstate();
	luaL_openlibs(LuaState);
	// export core classes
	REGISTER_CLASS(LuaState, GameObject);
	REGISTER_CLASS(LuaState, Scene);
	REGISTER_CLASS(LuaState, Level);
	REGISTER_CLASS(LuaState, Model);
	REGISTER_CLASS(LuaState, Material);
	REGISTER_CLASS(LuaState, MeshRenderer);
	REGISTER_CLASS(LuaState, InputSystem);
	REGISTER_CLASS(LuaState, PhysicsObject);
    REGISTER_CLASS(LuaState, Animator);
    REGISTER_CLASS(LuaState, BlendingNode);
}

int ScriptingSystem::Initialize() {
	InitEnvironment();
	// subscribe to level load events
	context->SubscribeFor(this, 400);
	// register objects
	context->RegisterObject<Script>();
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

	//push scene
	Scene * scene = level->GetScene();
	LuaStack::Push(LuaState, scene);
	lua_setglobal(LuaState, "scene");
	// push level
	LuaStack::Push(LuaState, level);
	lua_setglobal(LuaState, "level");
	// push Input
	LuaStack::Push(LuaState, context->GetSubsystem<InputSystem>());
	lua_setglobal(LuaState, "input");
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
	ret = lua_pcall(LuaState, 0, 1, 0);
	if (ret) {
		printf("error: %s\n", lua_tostring(LuaState, -1));
	}
    lua_settop(LuaState, 0);
	// read another line
	GetConsoleInput();
}

void ScriptingSystem::RegisterScript(Script * script) {
	script->AddRef();
	Scripts.Insert(script);
}

void ScriptingSystem::RemoveScript(Script * script) {
	Scripts.Remove(script);
	script->DecRef();
}

int ScriptingSystem::LoadFile(String& File) {
    int ret = 0;
    lua_getglobal(LuaState, "scripts");
    lua_getfield(LuaState, -1, File.ToStr());
    if (lua_isnil(LuaState, -1)) {
        // we need to load this scripts as a table
        printf("new file %s\n", File.ToStr());
        lua_pop(LuaState, 1);
        luaL_loadfile(LuaState, File.ToStr());
        if (ret) {
            printf("Couldn't load script: %s\n", lua_tostring(LuaState, -1));
            return -1;
        }
        lua_newtable(LuaState);
        // put newly created templte to "scripts" table at -4
        lua_pushvalue(LuaState, -1);
        lua_setfield(LuaState, -4, File.ToStr());
        // set upvalue for loaded scripts
        lua_setupvalue(LuaState, -2, 1);
        // run this scripts in template's ENV
        lua_pcall(LuaState, 0, 0, 0);
        if (ret) {
            printf("error pcall: %s\n", lua_tostring(LuaState, -1));
        }
        // push template on the top
        lua_getfield(LuaState, -1, File.ToStr());
    } else {
       // printf("file %s is already loaded\n", (char*)File);
    }
    // balance remove "scripts"
    lua_remove(LuaState, -2);
    return 0;
}

