#include "Include\HDRProcesser.h"

using namespace Render;

CHDRProcesser::CHDRProcesser(IMRender * Render):CProcesser(Render)
{
}


CHDRProcesser::~CHDRProcesser(void)
{
}


int Render::CHDRProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	//m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->SetProjectionMatrix(&State->Projection);
	m_Render->SetRenderContext(Command->ContextKey);
	m_Render->EnableHDRPass();
	return 0;
}

// process a renderable
int Render::CHDRProcesser::ProcessRenderable(Renderable * pRenderable,int * ContextKey)
{
	pRenderable->Node->Draw(m_Render,NORMAL_PASS,ContextKey,pRenderable->Param);
	return 0;
}