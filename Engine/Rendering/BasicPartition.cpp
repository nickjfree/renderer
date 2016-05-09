#include "BasicPartition.h"
#include "RenderingSystem.h"

BasicPartition::BasicPartition(Context * context) :PartitionMethod(context)
{
	BasicCulling = new BasicCullingImp();
	RenderingSystem * Render = context->GetSubsystem<RenderingSystem>();
	Render->SetSpatial(BasicCulling);
}


BasicPartition::~BasicPartition()
{
}

int BasicPartition::OnAttach(GameObject * GameObj) {
	// add quad tree to scene
	//SceneManager->ActiveQuadTree(Tree);
	return 0;
}

int BasicPartition::HandleEvent(Event * Evt) {
	// test code
	if (Evt->EventId == 300) {
		RenderObject * Node = Evt->EventParam["RenderObject"].as<RenderObject*>();
		//Tree->AddSenceNode(Node, 0);
		BasicCulling->Add(Node);
	}
	return 0;
}
