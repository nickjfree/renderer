#include "Include\SceneOccludee.h"

using namespace SceneManager;

CSceneOccludee::CSceneOccludee(void):m_Occludee(true)
{
}


CSceneOccludee::~CSceneOccludee(void)
{
}

int CSceneOccludee::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
	if(m_Mesh == -1)
			return -1;
	Render->SetWorldMatrix(&m_InstanceInfo.TransMatrix);
	Render->IssueOcclusionQuery(&m_Occludee);
	Render->RenderGeometry(ContextKey[CONTEXT_GEOMETRY]);
	Render->Flush();
	Render->EndOcclusionQuery();
	return 0;
}
