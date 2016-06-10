#include "Light.h"
#include "Scene\Scene.h"

USING_ALLOCATER(Light);
Light::Light(Context * context) :Component(context)
{
	renderLight = new RenderLight();
}


Light::~Light()
{
}

int Light::Load(void * Raw, Level * level) {
	LightEntry * Entry = (LightEntry*)Raw;
	Model * model = level->GetModel(Entry->ModelIndex);
	Material * material = level->GetMaterial(Entry->MaterialIndex);
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

int Light::OnAttach(GameObject * GameObj) {
	SubscribeTo(Owner, 200);
	Scene * scene = GameObj->GetScene();
	// Notify partition
	Event * Evt = Event::Create();
	Evt->EventId = 300;
	Evt->EventParam["RenderObject"].as<RenderObject*>() = renderLight;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// set init position
	renderLight->SetPosition(GameObj->GetTranslation());
	renderLight->SetRotation(GameObj->GetRotation());
	return Component::OnAttach(GameObj);
}


int Light::HandleEvent(Event * Ev) {
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

