
#include "Terrain.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"


USING_ALLOCATER(Terrain)

Terrain::Terrain(Context* context): Component(context), sizeX_(), sizeY_(), clipmapGeometry_()
{
}

// build terrain quad tree
void Terrain::Init(int sizeX, int sizeY, int numLevels)
{
	// only fixed size.  levels: 8  tileSize: 8.
	numLevels = 8;
	auto rootTileSize =  TerrainTileSize * (int)pow(2, numLevels - 1);
	sizeX = sizeY = 8192; // 4*4 rootNode for 4km * 4km terrain
	// init terrain
	// 1. calc node numbers
	int numNodes = 0;
	int currentLevel = 0;
	while (currentLevel < numLevels) {
		numNodes += (int)pow(4, currentLevel++);
	}
	// 2. calc rootNode numbers
	auto numRootNodeX = sizeX / rootTileSize;
	auto numRootNodeY = sizeY / rootTileSize;
	int numRootNodes = numRootNodeX * numRootNodeY;
	numNodes *= numRootNodes;

	// 3. init quad tree

	for (auto x = 0; x < numRootNodeX; x++) {
		for (auto y = 0; y < numRootNodeY; y++) {
			auto rootNode = CreateNode(x, y, numLevels - 1);
			rootNodes.PushBack(rootNode);
			
		}
	}
}

TerrainNode* Terrain::CreateNode(int x, int y, int level)
{
	// create current node
	auto node = context->CreateObject<TerrainNode>();
	// create culling geometry
	int currentSzie = TerrainTileSize * (int)pow(2, level);
	float d = 0.5f * currentSzie;
	Vector3 Center(0, 0, 0);
	auto cullingObject = AABB(Center, Vector3(d, 10, d));
	node->SetCullingObject(cullingObject);
	// set terrain (this)
	node->SetTerrain(this);
	// set level
	node->SetLevel(level);
	// set position
	node->SetPosition(Vector3(x + d, 0, y + d));
	if (level) {
		// not the finest level
		int childSize = TerrainTileSize * (int)pow(2, level - 1);
		// create 4 child node
		auto nodeUL = CreateNode(x, y, level - 1);
		auto nodeUR = CreateNode(x + childSize, y, level - 1);
		auto nodeBL = CreateNode(x, y + childSize, level - 1);
		auto nodeBR = CreateNode(x + childSize, y + childSize, level - 1);
		// add child node
		node->AddChild(nodeUL);
		node->AddChild(nodeUR);
		node->AddChild(nodeBL);
		node->AddChild(nodeBR);
	}
	// add to node list
	Nodes_.PushBack(node);
	return node;
}

int Terrain::OnAttach(GameObject* GameObj)
{
	Scene* scene = GameObj->GetScene();
	// Notify partition
	// add all rootNodes to scenen partition

	for (auto iter = rootNodes.Begin(); iter != rootNodes.End(); iter++) {
		Event* Evt = Event::Create();
		Evt->EventId = EV_NODE_ADD;
		Evt->EventParam["RenderObject"].as<Node*>() = *iter;
		SendEvent(scene, Evt);
		Evt->Recycle();
	}
	return 0; 
}

int Terrain::OnDestroy(GameObject* GameObj)
{
	// Notify partition to remove renderobject from scenegraph
	Scene* scene = Owner->GetScene();
	for (auto iter = rootNodes.Begin(); iter != rootNodes.End(); iter++) {
		// remove all the node
		auto node = *iter;
		// send event to remove it
		Event* Evt = Event::Create();
		Evt->EventId = EV_NODE_REMOVE;
		Evt->EventParam["RenderObject"].as<Node*>() = *iter;
		SendEvent(scene, Evt);
		Evt->Recycle();
	}
	// clear all node
	for (auto iter = Nodes_.Begin(); iter != Nodes_.End(); iter++) {
		delete *iter;
	}
	Component::OnDestroy(GameObj);
	return 0;
}

