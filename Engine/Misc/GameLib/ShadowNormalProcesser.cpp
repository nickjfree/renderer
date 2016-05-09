#include "Include\ShadowNormalProcesser.h"


using namespace Render;

CShadowNormalProcesser::CShadowNormalProcesser(IMRender* Render):CNormalProcesser(Render)
{
}


CShadowNormalProcesser::~CShadowNormalProcesser(void)
{
}

int CShadowNormalProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	//m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->LightMatrix[m_LightIndex],&State->ViewPoint);
	m_Render->SetProjectionMatrix(&State->LightProjection[m_LightIndex]);
	m_Render->SetRenderContext(Command->ContextKey);
	//m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableShadowPass(m_LightIndex);
	return 0;
}


int CShadowNormalProcesser::ProcessRenderable(Renderable * pRenderable,int * ContextKey)
{
	pRenderable->Node->Draw(m_Render,SHADOW_PASS,ContextKey,pRenderable->Param);
	return 0;
}
