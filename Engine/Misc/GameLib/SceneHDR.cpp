#include "Include\SceneHDR.h"



using namespace SceneManager;

CSceneHDR::CSceneHDR(void)
{
}


CSceneHDR::~CSceneHDR(void)
{
}


int SceneManager::CSceneHDR::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Render->PerformeHDR();
	return 0;
}
