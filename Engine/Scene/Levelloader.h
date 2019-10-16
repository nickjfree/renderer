#ifndef __LEVELLOADER__
#define __LEVELLOADER__

#include "Core\System.h"
#include "Scene\Scene.h"
#include "Scene\GameObject.h"
#include "Resource\ResourceCache.h"
/*
	Level Loader
*/
class LevelLoader : public System
{
	OBJECT(LevelLoader);
private:
	// actived levels
	List<Level> levels_;
	// test frames
	int test_frames;
public:
	LevelLoader(Context* context);
	virtual ~LevelLoader();

	virtual int Initialize();

	// update
	virtual int Update(int ms);

	// event hander
	virtual int HandleEvent(Event* event);

	// load level
	int LoadLevel(const String& path);

	// unload level
	int UnloadLevel(const String& path);
};

#endif