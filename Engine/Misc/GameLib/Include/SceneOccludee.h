#ifndef __SCENE_OCCLUDEE__
#define __SCENE_OCCLUDEE__

#include "SceneBasicGeometry.h"

namespace SceneManager{

class CSceneOccludee : public CSceneBasicGeometry
{
private:
	BOOL     m_Occludee;
public:
	CSceneOccludee(void);
	virtual ~CSceneOccludee(void);
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);
	virtual bool IsOccludee(){return !m_Occludee;};
};

}

#endif
