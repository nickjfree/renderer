#include "Include\NormalInstancingProcesser.h"

using namespace Render;

CNormalInstancingProcesser::CNormalInstancingProcesser(IMRender * Render):CProcesser(Render)
{
}

CNormalInstancingProcesser::~CNormalInstancingProcesser(void)
{
}

int CNormalInstancingProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableGeometryPass();
	return 0;
}

int CNormalInstancingProcesser::ProcessRenderable(Renderable * pRenderable)
{
	if(pRenderable->GeometryID == -1)
	{
		return -1;
	}
	m_Render->SetSkin(pRenderable->SkinID);
	m_Render->SetWorldMatrix(pRenderable->WorldMatrix);
	m_Render->UseBoneData(pRenderable->BoneMatrixID);
	m_Render->RenderInstance(0,pRenderable->VertexData,pRenderable->VSize,pRenderable->GeometryID);
	return 0;
}

CLPPNormalInstancingProcesser::CLPPNormalInstancingProcesser(IMRender * Render):CNormalInstancingProcesser(Render)
{
}

CLPPNormalInstancingProcesser::~CLPPNormalInstancingProcesser(void)
{
}

int CLPPNormalInstancingProcesser::Prepare(Render::RenderState *State, Render::RenderCommand *Command)
{
	m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableLightPrePass();
	return 0;
}

int CLPPNormalInstancingProcesser::ProcessRenderable(Renderable * pRenderable)
{
	pRenderable->Node->Refresh();
	return CNormalInstancingProcesser::ProcessRenderable(pRenderable);
}
