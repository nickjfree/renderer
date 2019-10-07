#include "BasicPartition.h"
#include "RenderingSystem.h"

USING_ALLOCATER(BasicPartition)

BasicPartition::BasicPartition(Context * context) :PartitionMethod(context)
{
    BasicCulling = new BasicCullingImp();
}


BasicPartition::~BasicPartition()
{
	delete BasicCulling;
}

int BasicPartition::OnAttach(GameObject * GameObj) {
	RenderingSystem* Render = context->GetSubsystem<RenderingSystem>();
	Render->SetSpatial(BasicCulling);
    return 0;
}

int BasicPartition::HandleEvent(Event * Evt) {
    // test code
    if (Evt->EventId == EV_NODE_ADD) {
        RenderObject * Node = Evt->EventParam["RenderObject"].as<RenderObject*>();
        //Tree->AddSenceNode(Node, 0);
        BasicCulling->Add(Node);
    }
    if (Evt->EventId == EV_NODE_REMOVE) {
        RenderObject * Node = Evt->EventParam["RenderObject"].as<RenderObject*>();
        //Tree->AddSenceNode(Node, 0);
        BasicCulling->Remove(Node);
    }
    EventNode::HandleEvent(Evt);
    return 0;
}
