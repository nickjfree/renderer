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
		LoadLevel(LevelUrl);
		flag = 1;
		return 0;
    }
  //  // try to get that resource 
  //  Level * level = Cache->Get<Level>(LevelUrl);
	//if (level && level->GetAsyncStatus() == Resource::S_ACTIVED) {
	//	printf("Level %s loaded\n", level->GetUrl().ToStr());
	//	// unload it
	//	Variant Param{};
	//	Cache->AsyncUnLoadResource(LevelUrl, NULL, Param);
	//}
	//if (!level || level->GetAsyncStatus() == Resource::S_DESTORYED) {
	//	Variant Param{};
	//	Cache->AsyncLoadResource(LevelUrl, NULL, Param);
	//}

	for (auto iter = levels_.Begin(); iter != levels_.End(); iter++) {
		auto level = *iter;
		if (!level->Destroyed) {
			level->Update(ms);
		} else {
			level->DestroyedFrames++;
		}
	}
	// handle level destroyed for more than 4 frames, safe to unload
	auto cache = context->GetSubsystem<ResourceCache>();
	Variant param{};
	auto iter = levels_.Begin();
	while (iter != levels_.End()) {
		auto level = *iter;
		if (level->DestroyedFrames >= 4) {
			levels_.Remove(iter);
			cache->AsyncUnLoadResource(level->GetUrl(), nullptr, param);
			iter = levels_.Begin();
		} else {
			iter++;
		}
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

// load level
int LevelLoader::LoadLevel(const String& path) {
	auto cache = context->GetSubsystem<ResourceCache>();
	// load level resource
	Variant Param{};
	cache->AsyncLoadResource(path, nullptr, Param);
	levels_.Insert(cache->Get<Level>(path));
	return 0;
}

// unload level
int LevelLoader::UnloadLevel(const String& path) {
	auto cache = context->GetSubsystem<ResourceCache>();
	auto level = cache->Get<Level>(path);
	level->Destroy();
	return 0;
}
