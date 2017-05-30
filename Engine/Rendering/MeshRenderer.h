#ifndef __MESH_RENDERER__
#define __MESH_RENDERER__

#include "Renderer.h"
#include "RenderingSystem.h"
/*
	H3D mesh renderer
*/

#define MAX_LODS 4

class MeshRenderer : public Renderer {

	// macros
	OBJECT(MeshRenderer);
	BASEOBJECT(Renderer);
	DECLAR_ALLOCATER(MeshRenderer);
//	UUID();

private:
	// render object
	RenderObject * renderObject;
	// h3d entity as mesh to render
	//CEntity * Entity[MAX_LODS];
	//// h3d marterial
	//CMaterial * Material[MAX_LODS];
	//// H3D RenderEngine SceneNode as Renderer
	//CSceneObject * SceneObject;
	//// H3D RenderEngine Reference
	//CRenderEngine * RenderEngine;
	//// H3D Scene Manager
	//CSceneManager * SceneManager;
	// current lod level
	int Lod;
private:
	// setup
	void Init();

public:
	MeshRenderer(Context * context);
	virtual ~MeshRenderer();
	// on attach
	virtual int OnAttach(GameObject * GameObj);
	// transform change
	virtual int MeshRenderer::OnTransform(GameObject * GameObj);
	// load
	virtual int Load(void * Raw, Level * level);
	// Handler event
	virtual int HandleEvent(Event * Ev);
	// set model
	void SetModel(Model * model) { renderObject->SetModel(model); }
	// set material
	void SetMaterial(Material * material) { renderObject->SetMaterial(material); }
};


#endif