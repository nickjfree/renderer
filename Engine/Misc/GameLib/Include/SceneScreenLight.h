#ifndef __SCREENLIGHT__
#define __SCREENLIGHT__


#include "scenelight.h"


namespace SceneManager {

class CSceneScreenLight :public CSceneLight
{
public:
	CSceneScreenLight(void);
	virtual ~CSceneScreenLight(void);

public:
	virtual void Refresh();
	virtual int Render(int LOD);
	virtual int DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter);
};


} //end namespace


#endif