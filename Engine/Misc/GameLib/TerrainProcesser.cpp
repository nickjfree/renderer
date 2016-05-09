#include "Include\TerrainProcesser.h"

using namespace Render;

CTerrainProcesser::CTerrainProcesser(IMRender * Render):CNormalSolidProcesser(Render)
{
}

CTerrainProcesser::~CTerrainProcesser(void)
{
}

int CTerrainProcesser::ProcessRenderable(Renderable * pRenderable)
{
	m_Render->SetSkin(pRenderable->SkinID);
	m_Render->Render(1,pRenderable->VertexData,pRenderable->VSize,pRenderable->IndexData,pRenderable->INum);
	return 0;
}

CLPPTerrainProcesser::CLPPTerrainProcesser(IMRender * Render):CTerrainProcesser(Render)
{
}

CLPPTerrainProcesser::~CLPPTerrainProcesser(void)
{
}

int CLPPTerrainProcesser::Prepare(Render::RenderState *State, Render::RenderCommand *Command)
{
	m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableLightPrePass();
	return 0;
}
