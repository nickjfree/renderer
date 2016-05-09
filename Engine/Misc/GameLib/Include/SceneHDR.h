#ifndef  __SCENE_HDR__
#define  __SCENE_HDR__

#include "scenesprite.h"

namespace SceneManager {

class CSceneHDR : public CSceneSprite
{
public:
	CSceneHDR(void);
	~CSceneHDR(void);

public:
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);
};


} // end namespace



#endif
