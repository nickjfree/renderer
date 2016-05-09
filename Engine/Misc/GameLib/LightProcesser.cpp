#include "Include\LightProcesser.h"
#include "SceneLight.h"

using namespace Render;
using namespace SceneManager;

CLightProcesser::CLightProcesser(IMRender * Render):CProcesser(Render)
{
}

CLightProcesser::~CLightProcesser(void)
{
}

int Render::CLightProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	//m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->SetProjectionMatrix(&State->Projection);
	m_Render->SetRenderContext(Command->ContextKey);
	m_Render->EnableLightingPass();
	return 0;
}

// process a renderable
int Render::CLightProcesser::ProcessRenderable(Renderable * pRenderable,int * ContextKey)
{
	LightRenderParameter * LightParam = (LightRenderParameter *)pRenderable->Param;
	LightParam->LightViewMatrix = m_RenderState->LightMatrix[LightParam->LightIndex];
	LightParam->LightProjection = m_RenderState->LightProjection[LightParam->LightIndex];
	pRenderable->Node->Draw(m_Render,LIGHTPRE_PASS,ContextKey,pRenderable->Param);
	return 0;
}
