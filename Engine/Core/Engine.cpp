#include "Engine.h"

#include "Rendering\RenderingSystem.h"
#include "Tasks\WorkQueue.h"
#include "Resource\ResourceCache.h"
#include "Scene\Levelloader.h"



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
	// more post tasks
	Render->PreloadingResource();
	return 0;
}

int Engine::Update(int ms) {
	RenderingSystem * Render = context->GetSubsystem<RenderingSystem>();
	LevelLoader * Level = context->GetSubsystem<LevelLoader>();
	WorkQueue * Queue = context->GetSubsystem<WorkQueue>();
	Queue->Update(ms);
	Level->Update(ms);
	Render->Update(ms);
	return 0;
}

int Engine::Shutdown() {
	return 0;
}