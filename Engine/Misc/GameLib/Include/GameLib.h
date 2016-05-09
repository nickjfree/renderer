#ifndef __GAME_LIB__
#define __GAME_LIB__

#include "IMRender.h"
#include "GameCamera.h"
#include "QuadTree.h"
#include "render.h"
#include "ResourceManager.h"

using SceneManager::CQuadTree;
using Render::CRender;
using ResourceManager::CResourceManager;


IMRender * CreateMoonRender();
CQuadTree * CreateQuadTree();
CQuadTree * GetQuadTree();
CRender   * CreateRender();
CRender   * GetRender();
CResourceManager * CreateResourceManager();


#endif