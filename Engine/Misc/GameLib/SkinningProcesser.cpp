#include "Include\SkinningProcesser.h"
#include "SceneSoftBody.h"

using namespace Render;
using SceneManager::SoftBodyData;

CSkinningProcesser::CSkinningProcesser(IMRender * Render):CNormalSolidProcesser(Render)
{
}

CSkinningProcesser::~CSkinningProcesser(void)
{
}

int CSkinningProcesser::ProcessRenderable(Render::Renderable *pRenderable)
{
	SoftBodyData * Data = (SoftBodyData*)pRenderable->Param;
	m_Render->SetSkin(pRenderable->SkinID);
	m_Render->SetSkinningMatrix(Data->SkinningMatrixRender,Data->Bones);
	m_Render->UsePass(1,3);
	m_Render->RenderGeometry(pRenderable->GeometryID);
	m_Render->Flush();
	return 0;
}

CLPPSkinningProcesser::CLPPSkinningProcesser(IMRender * Render):CSkinningProcesser(Render)
{
}

CLPPSkinningProcesser::~CLPPSkinningProcesser(void)
{
}

int CLPPSkinningProcesser::Prepare(Render::RenderState *State, Render::RenderCommand *Command)
{
	m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableLightPrePass();
	return 0;
}

int CLPPSkinningProcesser::ProcessRenderable(Render::Renderable *pRenderable)
{
	pRenderable->Node->Refresh();
	SoftBodyData * Data = (SoftBodyData*)pRenderable->Param;
	m_Render->SetSkin(pRenderable->SkinID);
	m_Render->SetSkinningMatrix(Data->SkinningMatrixRender,Data->Bones);
	m_Render->UsePass(1,2);
	m_Render->RenderGeometry(pRenderable->GeometryID);
	m_Render->Flush();
	return 0;
}
