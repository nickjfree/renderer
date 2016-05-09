#ifndef __SCENE_INSTANCING__
#define __SCENE_INSTANCING__

#include "SceneObject.h"
#include "Render.h"
#include "Entity.h"

using SceneManager::CSceneNode;
using SceneManager::CEntity;
using Render::CRender;

namespace SceneManager{



class CSceneInstancing : public CSceneObject
{
public:
	CSceneInstancing(void);
	virtual ~CSceneInstancing(void);
//	virtual void Refresh();
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);
//	virtual int DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter);
//	virtual int DrawShadowPass(IMRender * Render,int * ContextKey,void * RenderParameter);
};


}//end namespace

#endif