
#include "TerrainNode.h"
#include "Terrain.h"

USING_ALLOCATER(TerrainNode)


bool TerrainNode::IsVisible(const Vector3& viewPoint)
{
	float distanceX = abs(viewPoint.x - Position.x);
	float distanceY = abs(viewPoint.z - Position.z);
	return (distanceX <= viewDistance_ && distanceY <= viewDistance_);
}

TerrainNode::TerrainNode(Context* context) : Resource(context), level_(), offsetX_(), offsetY_(), scale_(),
terrainPayload_(), payloadState_(), terrain_(), viewDistance_()
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

int TerrainNode::Query(Frustum& Fr, Vector<Node*>& Result, int Types, bool inside) {
	// test if we are the one
	if (Node::Type & Types) {
		Node* node = this;
		Result.PushBack(node);
	} else {
		return 1;
	}
	// get view point
	Vector3 viewPoint = Fr.Center;


	// child nodes
	for (auto Iter = SubNodes.Begin(); Iter != SubNodes.End(); Iter++) {
		TerrainNode* sub = static_cast<TerrainNode*>(*Iter);
		auto& cull_obj = sub->GetCullingObject();
		int test = Fr.Contains(cull_obj);
		switch (test) {
		case ContainType::CONTAINS:
			inside = true;
			break;
		case ContainType::INTERSECTS:
			inside = false;
			break;
		case ContainType::DISJOINT:
			continue;
		default:
			continue;
		}
		// more details needed
		if (sub->IsVisible(viewPoint)) {
			sub->Query(Fr, Result, Types, inside);
		}
	}
	return 0;
}

int TerrainNode::Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context)
{
	StageParameter["InstancePosition"].as<Vector3>() = Position;
	StageParameter["InstanceScale"].as<float>() = (float)scale_;
	StageParameter["InstanceLevel"].as<float>() = (float)level_;

	// do virtual texture rendering
	Stage = 3;
	if (material) {
		auto compiled = material->Compile(Compiler, Stage, Lod);
		// process shader
		auto shader = material->GetShader();
		compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), StageParameter, Context);
		if (shader->IsInstance(Stage)) {
			unsigned char instanceBuffer[64 * 4];
			auto instanceSize = shader->MakeInstance(Compiler, Stage, StageParameter, instanceBuffer);
			// quad
			compiled += Compiler->Instance(0, instanceBuffer, instanceSize);
		}
		return compiled;
	}
	return 0;
}


void TerrainNode::SetLevel(int level) 
{
	level_ = level;
	// get view distance
	viewDistance_ = TerrainFinestLevelViewDistance * (float)pow(2, level) + TerrainTileSize * (float)pow(2, level-1);
	// scale
	scale_ = TerrainTileSize * (float)pow(2, level);
}