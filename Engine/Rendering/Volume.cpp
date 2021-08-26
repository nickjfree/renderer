#include "Volume.h"
#include "GIVolume.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"

USING_ALLOCATER(Volume);

Volume::Volume(Context* context): Component(context)
{
	// defualt to gi volume for test
	volumeNode = new GIVolume();
}


Volume::~Volume()
{
	if (volumeNode) {
		delete volumeNode;
		volumeNode = nullptr;
	}
}


int Volume::OnAttach(GameObject* GameObj)
{
	auto scene = GameObj->GetScene();
	// Notify partition
	Event* Evt = Event::Create();
	Evt->EventId = EV_NODE_ADD;
	Evt->EventParam["RenderObject"] = volumeNode;
	SendEvent(scene, Evt);
	Evt->Recycle();
	// set volume position and size
	volumeNode->SetPosition(GameObj->GetWorldTranslation());
	return Component::OnAttach(GameObj);
}


int Volume::OnTransform(GameObject* GameObj) {
	// set init position
	volumeNode->SetPosition(GameObj->GetWorldTranslation());
	volumeNode->SetRotation(GameObj->GetWorldRotation());
	return 0;
}
