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
	DECLARE_ALLOCATER(MeshRenderer);
	//	UUID();

private:
	// render object
	RenderObject* renderObject;
	// current lod level
	int Lod;
private:
	// setup
	void Init();

public:
	MeshRenderer(Context* context);
	virtual ~MeshRenderer();
	// on attach
	virtual int OnAttach(GameObject* GameObj);
	// on destroy
	virtual int OnDestroy(GameObject* GameObj);
	// transform change
	virtual int MeshRenderer::OnTransform(GameObject* GameObj);
	// load
	virtual int Load(void* Raw, Level* level);
	// save
	virtual int Save(Serializer* saveFile, Level* level);
	// set model
	void SetModel(Model* model) { renderObject->SetModel(model); }
	// set material
	void SetMaterial(Material* material) { renderObject->SetMaterial(material); }
	// set transparent
	void SetTransparente();
	// get model
	Model* GetModel() { return renderObject->GetModel(); }
	// get test frame data
	void SetMatrixPalette(Matrix4x4* palette, unsigned int NumMatrix);
	// set blendshape
	void SetBlendShape(BlendShape* Shape);
	// set blendshape desc
	void SetBlendShapeDesc(BSDesc* desc);
};


#endif