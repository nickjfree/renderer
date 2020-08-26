
#include "TerrainNode.h"


USING_ALLOCATER(TerrainNode)


TerrainNode::TerrainNode(Context* context) : Resource(context), level_(), offsetX_(), offsetY_(), scale_(), terrainPayload_(), payloadState_()
{
	Node::Type = Node::TERRAIN;
}


int TerrainNode::OnSubResource(int Message, Resource* Sub, Variant& Param)
{
	// set state to paged_in
	payloadState_ = PayloadState::PAGED_IN;
	// update the indirection texture in cpu mem. 
	 
	return 0;
}
