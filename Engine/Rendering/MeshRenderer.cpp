#include "MeshRenderer.h"
#include "Scene\GameObject.h"
#include "Scene\Scene.h"
#include "H3DMesh.h"
#include "Core\StringTable.h"


USING_ALLOCATER(MeshRenderer);

MeshRenderer::MeshRenderer(Context* context) : Renderer(context) {
	// init component
	Init();
}


MeshRenderer::~MeshRenderer() {
	// free renderObj
	delete renderObject;
}

void MeshRenderer::Init() {
	renderObject = new RenderObject();
}

int MeshRenderer::OnAttach(GameObject* GameObj) {
	Scene* scene = GameObj->GetScene();
	// Notify partition
	Event* Evt = Event::Create();
	Evt->EventId = EV_NODE_ADD;
	Evt->EventParam[hash_string::RenderObject].as<RenderObject*>() = renderObject;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// set init position
	renderObject->SetPosition(GameObj->GetWorldTranslation());
	renderObject->SetRotation(GameObj->GetWorldRotation());
	return Component::OnAttach(GameObj);
}

int MeshRenderer::OnDestroy(GameObject* GameObj) {
	// Notify partition to remove renderobject from scenegraph
	Scene* scene = Owner->GetScene();
	Event* Evt = Event::Create();
	Evt->EventId = EV_NODE_REMOVE;
	Evt->EventParam[hash_string::RenderObject].as<RenderObject*>() = renderObject;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// call base destroy
	Renderer::OnDestroy(GameObj);
	return 0;
}

int MeshRenderer::Load(void* Raw, Level* level) {
	RenderEntry* Data = (RenderEntry*)Raw;
	Model* model = level->GetModel(Data->ModelIndex);
	Material* material = level->GetMaterial(Data->MaterialIndex);
	SetModel(model);
	SetMaterial(material);
	return sizeof(RenderEntry);
}

int MeshRenderer::Save(Serializer* saveFile, Level* level) {
	RenderEntry Entry;
	strcpy_s(Entry.Info.TypeName, "Renderer");
	Entry.ModelIndex = renderObject->GetModel()->LevelId;
	Entry.MaterialIndex = renderObject->GetMaterial()->LevelId;
	Entry.pad1 = Entry.pad2 = 0;
	saveFile->Write(&Entry);
	return 0;
}

int MeshRenderer::OnTransform(GameObject* GameObj) {
	// set init position
	renderObject->SetPosition(GameObj->GetWorldTranslation());
	renderObject->SetRotation(GameObj->GetWorldRotation());
	return 0;
}

void MeshRenderer::SetMatrixPalette(Matrix4x4* palette, unsigned int NumMatrix) {
	renderObject->SetMatrixPalette(palette, NumMatrix);
}

void MeshRenderer::SetTransparente() {
	renderObject->SetTransparent();
}

void MeshRenderer::SetClipmap() {
	renderObject->SetClipmap();
}

void MeshRenderer::SetBlendShape(BlendShape* Shape) {
	renderObject->SetBlendShape(Shape);
}

void MeshRenderer::SetBlendShapeDesc(BSDesc* desc) {
	renderObject->SetBlendShapeDesc(desc);
}