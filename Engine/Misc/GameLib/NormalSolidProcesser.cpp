#include "Include\NormalSolidProcesser.h"

using namespace Render;

CNormalSolidProcesser::CNormalSolidProcesser(IMRender * Render):CDefaultProcesser(Render)
{
}

CNormalSolidProcesser::~CNormalSolidProcesser(void)
{
}

int CNormalSolidProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableGeometryPass();
	return 0;
}

int CNormalSolidProcesser::ProcessRenderable(Renderable * pRenderable)
{
	m_Render->SetSkin(pRenderable->SkinID);
	m_Render->SetWorldMatrix(pRenderable->WorldMatrix);
	m_Render->UseBoneData(pRenderable->BoneMatrixID);
	m_Render->Render(0,pRenderable->VertexData,pRenderable->VSize,pRenderable->IndexData,pRenderable->INum);
	return 0;
}

CLPPNormalSolidProcesser::CLPPNormalSolidProcesser(IMRender * Render):CNormalSolidProcesser(Render)
{
}

CLPPNormalSolidProcesser::~CLPPNormalSolidProcesser(void)
{
}


int CLPPNormalSolidProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableLightPrePass();
	return 0;
}

int CLPPNormalSolidProcesser::ProcessRenderable(Render::Renderable *pRenderable)
{
	pRenderable->Node->Refresh();
	return CNormalSolidProcesser::ProcessRenderable(pRenderable);
}
