#include "Include\PostProcesser.h"


using namespace Render;

CPostProcesser::CPostProcesser(IMRender * Render):CProcesser(Render)
{
}


CPostProcesser::~CPostProcesser(void)
{
}


int Render::CPostProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	//m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->SetProjectionMatrix(&State->Projection);
	m_Render->SetRenderContext(Command->ContextKey);
	m_Render->EnablePostPass();
	return 0;
}

// process a renderable
int Render::CPostProcesser::ProcessRenderable(Renderable * pRenderable,int * ContextKey)
{
	pRenderable->Node->Draw(m_Render,NORMAL_PASS,ContextKey,pRenderable->Param);
	return 0;
}