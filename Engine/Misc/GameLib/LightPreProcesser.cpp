#include "Include\LightPreProcesser.h"

using namespace Render;

CLightPreProcesser::CLightPreProcesser(IMRender * Render):CProcesser(Render)
{
}


CLightPreProcesser::~CLightPreProcesser(void)
{
}

int CLightPreProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	//m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->SetProjectionMatrix(&State->Projection);
	m_Render->SetRenderContext(Command->ContextKey);
	//m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableLightPrePass();
	return 0;
}


int CLightPreProcesser::ProcessRenderable(Renderable * pRenderable,int * ContextKey)
{
	pRenderable->Node->Draw(m_Render,LIGHTPRE_PASS,ContextKey,pRenderable->Param);
	return 0;
}