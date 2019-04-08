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
    Scene * scene;
public:
    LevelLoader(Context * context);
    virtual ~LevelLoader();

    virtual int Initialize();

    // update
    virtual int Update(int ms);

    // event hander
    virtual int HandleEvent(Event * event);
};

#endif