#include "Light.h"
#include "Scene\Scene.h"

USING_ALLOCATER(Light);
Light::Light(Context* context):Component(context)
{
	renderLight = new RenderLight();
}


Light::~Light() {
	Scene* scene = Owner->GetScene();
	Event* Evt = Event::Create();
	Evt->EventId = EV_NODE_REMOVE;
	Evt->EventParam["RenderObject"].as<RenderObject*>() = renderLight;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// delete the node
	delete renderLight;
}

int Light::Load(void* Raw, Level* level) {
	LightEntry* Entry = (LightEntry*)Raw;
	Model* model = level->GetModel(Entry->ModelIndex);
	Material* material = level->GetMaterial(Entry->MaterialIndex);
	SetModel(model);
	SetMaterial(material);
	SetRadius(Entry->Radius);
	SetColor(Entry->Color);
	SetIntensity(Entry->Intensity);
	SetDirection(Entry->Direction);
	SetShadowCast(0);
	SetType(Entry->Type);
	return sizeof(LightEntry);
}

int Light::Save(Serializer* levelFile, Level* level) {
	LightEntry Entry{};
	strcpy_s(Entry.Info.TypeName, "Light");
	Entry.MaterialIndex = renderLight->GetMaterial()->LevelId;
	Entry.ModelIndex = renderLight->GetModel()->LevelId;
	Entry.Radius = renderLight->GetRadius();
	Entry.Color = renderLight->GetColor();
	Entry.Direction = renderLight->GetDirection();
	Entry.Type = renderLight->GetLightType();
	Entry.Intensity = renderLight->GetIntensity();
	levelFile->Write(&Entry);
	return 0;
}


int Light::OnAttach(GameObject* GameObj) {
	SubscribeTo(Owner, 200);
	Scene* scene = GameObj->GetScene();
	// Notify partition
	Event* Evt = Event::Create();
	Evt->EventId = EV_NODE_ADD;
	Evt->EventParam["RenderObject"].as<RenderLight*>() = renderLight;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// set init position
	renderLight->SetPosition(GameObj->GetWorldTranslation());
	renderLight->SetRotation(GameObj->GetWorldRotation());
	return Component::OnAttach(GameObj);
}

int Light::OnTransform(GameObject* GameObj) {
	// set init position
	renderLight->SetPosition(GameObj->GetWorldTranslation());
	renderLight->SetRotation(GameObj->GetWorldRotation());
	return 0;
}


int Light::HandleEvent(Event* Ev) {
	return 0;
}

void Light::SetRadius(float Radius) {
	renderLight->SetRadius(Radius);
}

void Light::SetShadowCast(int Flag) {
	renderLight->SetShadowCast(Flag);
}

void Light::SetColor(Vector3& Color) {
	renderLight->SetColor(Color);
}

void Light::SetIntensity(float intensity) {
	renderLight->SetIntensity(intensity);
}

void Light::SetDirection(Vector3& Direction) {
	renderLight->SetDirection(Direction);
}

