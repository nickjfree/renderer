#include "Engine.h"

#include "Rendering\RenderingSystem.h"
#include "Tasks\WorkQueue.h"
#include "Resource\ResourceCache.h"
#include "Scene\Levelloader.h"
#include "Script\Proxy.h"
#include "Script\ScriptingSystem.h"
#include "Input\InputSystem.h"
#include "Physics\PhysicsSystem.h"
#include "Animation\AnimationSystem.h"


Engine::Engine()
{
}


Engine::~Engine()
{
}

int Engine::Initialize(void) {
    context = new Context();
    //LuaStack::Push(NULL, context);
    return 0;
}

int Engine::InitSubsystems() {
    // WorkQueue
    WorkQueue * Queue = context->RegisterSubsystem<WorkQueue>();
    // ResourceCache
    ResourceCache * Resource = context->RegisterSubsystem<ResourceCache>();
    // Register rendering system
    RenderingSystem * Render = context->RegisterSubsystem<RenderingSystem>();
    // Level
    LevelLoader * Level = context->RegisterSubsystem<LevelLoader>();
	// input
	InputSystem* Input = context->RegisterSubsystem<InputSystem>();
    // scripting
    ScriptingSystem * Script = context->RegisterSubsystem<ScriptingSystem>();
    // physics
    PhysicsSystem * Physics = context->RegisterSubsystem<PhysicsSystem>();
    // animation
    AnimationSystem * Anime = context->RegisterSubsystem<AnimationSystem>();
    // more post tasks
    Render->PreloadingResource();
    return 0;
}

int Engine::Update(int ms) {
    RenderingSystem * Render = context->GetSubsystem<RenderingSystem>();
    LevelLoader * Level = context->GetSubsystem<LevelLoader>();
    WorkQueue * Queue = context->GetSubsystem<WorkQueue>();
    ScriptingSystem * Script = context->GetSubsystem<ScriptingSystem>();
    InputSystem * Input = context->GetSubsystem<InputSystem>();
    PhysicsSystem * Physics = context->GetSubsystem<PhysicsSystem>();
    AnimationSystem * Anime = context->GetSubsystem<AnimationSystem>();
    Queue->Update(ms);
    Level->Update(ms);
    Anime->Update(ms);
    Physics->Update(ms);
    Script->Update(ms);
    Render->Update(ms);
    Input->Update(ms);
    //	Sleep(5);
    return 0;
}

int Engine::OnMessage(int msg, size_t lParam, size_t wParam) {
    InputSystem * Input = context->GetSubsystem<InputSystem>();
    return Input->OnWindowsMessage(msg, lParam, wParam);
}

int Engine::Shutdown() {
    return 0;
}