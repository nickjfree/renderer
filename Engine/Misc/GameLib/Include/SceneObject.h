#ifndef __SCENE_OBJECT__
#define __SCENE_OBJECT__


#include "SceneNode.h"
#include "Render.h"
#include "Entity.h"

using SceneManager::CSceneNode;
using SceneManager::CEntity;
using Render::CRender;

namespace SceneManager{




class CSceneObject : public CSceneNode
{
public:
	CSceneObject(void);
	virtual ~CSceneObject(void);
	virtual int Render(int LOD);
	virtual int RenderShadow(int LOD, int Index);
	virtual int RenderLightPrePass();
	int SetProgress(float Progress);
	virtual void Refresh();
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);
	virtual int DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter);
	virtual int DrawShadowPass(IMRender * Render,int * ContextKey,void * RenderParameter);
};


}//end namespace

#endif