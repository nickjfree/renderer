
#include "TerrainNode.h"


USING_ALLOCATER(TerrainNode)


TerrainNode::TerrainNode(Context* context) : Resource(context)
{

}


int TerrainNode::OnSubResource(int Message, Resource* Sub, Variant& Param)
{
	// set state to paged_in
	payloadState_ = PayloadState::PAGED_IN;
	// update the indirection texture in cpu mem. 

	return 0;
}
