#include "Include\SceneScreenLight.h"


using namespace SceneManager;

CSceneScreenLight::CSceneScreenLight(void)
{
}


CSceneScreenLight::~CSceneScreenLight(void)
{
}

int CSceneScreenLight::Render(int LOD)
{
	if (m_Material)
	{
		CRender::GetRender()->AddRenderable(NULL, m_Material[LOD], this);
	}
	return 0;
}

void CSceneScreenLight::Refresh()
{
	m_Position.x = m_GlobalTrans._41;
	m_Position.y = m_GlobalTrans._42;
	m_Position.z = m_GlobalTrans._43;
	m_Position.w = 0;
}

int CSceneScreenLight::DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();  // update required values
	Render->SetRenderContext(ContextKey);
	Render->SetWorldMatrix(&m_LightSimpleData.Transform);
	m_Position.w = 0;
	Render->SetLightParameter(&m_Position, &m_Color, m_Radius, 0.3, -1);
	Render->PostProcess();
	Render->Flush();
	return 0;
}
