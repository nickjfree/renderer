#include "Levelloader.h"
#include "Rendering\QuadTree.h"
#include "Scene\TestComponent.h"
#include "Rendering\Mesh.h"
#include "Rendering\MeshRenderer.h"
#include "Rendering\Model.h"
#include "Resource\Level.h"
#include "Rendering\Light.h"


LevelLoader::LevelLoader(Context * context) : System(context)
{
}


LevelLoader::~LevelLoader()
{
}

int LevelLoader::Update(int ms) {
	// test code
	static int flag = 0;
	ResourceCache * Cache = context->GetSubsystem<ResourceCache>();
	String LevelUrl = "Level\\levels\\desktop.level\\0";
	if (!flag) {
		Variant Param;
		Cache->AsyncLoadResource(LevelUrl, NULL, Param);
		flag = 1;
	}
	// try to get that resource 
	Level * level = Cache->Get<Level>(LevelUrl);
	if (level) {
		level->Update(ms);
	}
	return 0;
}

int LevelLoader::HandleEvent(Event * event) {
	return 0;
}

int LevelLoader::Initialize() {
	context->RegisterObject<MeshRenderer>();
	context->RegisterObject<Light>();
	return 0;
}
