//#include <stdafx.h>
#include "windows.h"
#include "IMRender.h"
#include "GameCamera.h"
#include "QuadTree.h"
#include "MathLib.h"
#include "Render.h"
#include "ResourceManager.h"


using SceneManager::CQuadTree;
using Render::CRender;
using ResourceManager::CResourceManager;


typedef IMRender * ( *IFunc)(HINSTANCE hInst);

CGameCamera * gGameCamera;
CQuadTree   * gQuadTree;
CRender     * gRender;


IFunc GetMoonRender;


IMRender * CreateMoonRender()
{
	IMRender * Render;
	HINSTANCE hInst = LoadLibrary(L"GraphicRender.dll");
	GetMoonRender = (IFunc)GetProcAddress(hInst,"GetMoonRender");
	Render = GetMoonRender(hInst);
	return Render;
}



CGameCamera * GetGameCamera()
{
	return gGameCamera;
}

CQuadTree * CreateQuadTree()
{
	gQuadTree = new CQuadTree;
	return gQuadTree;
}

CQuadTree * GetQuadTree()
{
	return gQuadTree;
}

CRender   * GetRender()
{
	return gRender;
}

CRender   * CreateRender()
{
	return new CRender();
}

CResourceManager * CreateResourceManager()
{
	return new CResourceManager();
}