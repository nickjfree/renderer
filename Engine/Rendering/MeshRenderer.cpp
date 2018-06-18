#include "MeshRenderer.h"
#include "Scene\GameObject.h"
#include "Scene\Scene.h"
#include "H3DMesh.h"
#include "Core\StringTable.h"


USING_ALLOCATER(MeshRenderer);

MeshRenderer::MeshRenderer(Context * context) : Renderer(context) {
	// init component
	Init();
}


MeshRenderer::~MeshRenderer() {
	// Notify partition
	Scene * scene = Owner->GetScene();
	Event * Evt = Event::Create();
	Evt->EventId = EV_NODE_REMOVE;
	Evt->EventParam[hash_string::RenderObject].as<RenderObject*>() = renderObject;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// free renderObj
	delete renderObject;
}

void MeshRenderer::Init() {
	renderObject = new RenderObject();
}

int MeshRenderer::OnAttach(GameObject * GameObj) {
	SubscribeTo(Owner, 200);
	Scene * scene = GameObj->GetScene();
	// Notify partition
	Event * Evt = Event::Create();
	Evt->EventId = EV_NODE_ADD;
	Evt->EventParam[hash_string::RenderObject].as<RenderObject*>() = renderObject;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// set init position
	renderObject->SetPosition(GameObj->GetWorldTranslation());
	renderObject->SetRotation(GameObj->GetWorldRotation());
	return Component::OnAttach(GameObj);
}

int MeshRenderer::HandleEvent(Event * Ev) {
	// test code for now
	if (Ev->EventId == 200) {
		Model * model = Ev->EventParam[hash_string::model].as<Model*>();
		renderObject->SetModel(model);
	}
	return 0;
}

int MeshRenderer::Load(void * Raw, Level * level) {
	RenderEntry * Data = (RenderEntry*)Raw;
	Model * model = level->GetModel(Data->ModelIndex);
	Material * material = level->GetMaterial(Data->MaterialIndex);
	SetModel(model);
	SetMaterial(material);
	return sizeof(RenderEntry);
}

int MeshRenderer::OnTransform(GameObject * GameObj) {
	// set init position
	renderObject->SetPosition(GameObj->GetWorldTranslation());
	renderObject->SetRotation(GameObj->GetWorldRotation());
	return 0;
}

void MeshRenderer::SetMatrixPalette(Matrix4x4 * palette, unsigned int NumMatrix) {
	renderObject->SetMatrixPalette(palette, NumMatrix);
}