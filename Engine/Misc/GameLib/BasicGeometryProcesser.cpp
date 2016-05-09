#include "Include\BasicGeometryProcesser.h"

using namespace Render;

CBasicGeometryProcesser::CBasicGeometryProcesser(IMRender * Render):CNormalInstancingProcesser(Render)
{
}

CBasicGeometryProcesser::~CBasicGeometryProcesser(void)
{
}

int CBasicGeometryProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	m_Render->EnableLightingPass();
	return 0;
}
