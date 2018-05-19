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
	return 0;
}

int BasicPartition::HandleEvent(Event * Evt) {
	// test code
	if (Evt->EventId == EV_NODE_ADD) {
		RenderObject * Node = Evt->EventParam[String("RenderObject")].as<RenderObject*>();
		//Tree->AddSenceNode(Node, 0);
		BasicCulling->Add(Node);
	}
	if (Evt->EventId == EV_NODE_REMOVE) {
		RenderObject * Node = Evt->EventParam[String("RenderObject")].as<RenderObject*>();
		//Tree->AddSenceNode(Node, 0);
		BasicCulling->Remove(Node);
	}
	EventNode::HandleEvent(Evt);
	return 0;
}
