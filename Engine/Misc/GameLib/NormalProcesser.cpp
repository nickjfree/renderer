#include "Include\NormalProcesser.h"

using namespace Render;

CNormalProcesser::CNormalProcesser(IMRender* Render):CProcesser(Render)
{
}


CNormalProcesser::~CNormalProcesser(void)
{
}


int CNormalProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	//m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->SetProjectionMatrix(&State->Projection);
	m_Render->SetRenderContext(Command->ContextKey);
	//m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableGeometryPass();
	return 0;
}


int CNormalProcesser::ProcessRenderable(Renderable * pRenderable,int * ContextKey)
{
	pRenderable->Node->Draw(m_Render,NORMAL_PASS,ContextKey,pRenderable->Param);
	return 0;
}
