#include "Include\SceneInstancing.h"

using namespace SceneManager;


CSceneInstancing::CSceneInstancing()
{
}


CSceneInstancing::~CSceneInstancing()
{
}


int SceneManager::CSceneInstancing::DrawNormalPass(IMRender * Render, int * ContextKey, void * RenderParameter)
{
	Refresh();
	CEntity * Entity = GetEntity(0);
	Render->Render(0, &m_InstanceInfo, sizeof(m_InstanceInfo), NULL, 0);
	return 0;
}