#include "Node.h"


USING_ALLOCATER(Node);

Node::Node() :Type(Node::NODE), DeformableBuffer(-1), RaytracingGeometry(-1)
{
	SubNodes.Owner = this;
	Sibling.Owner = this;
}


Node::~Node()
{
}

void Node::AddChild(Node* Child) {
	Child->Sibling.InsertAfter(&SubNodes);
}

void Node::Remove(Node* Child) {
	Child->Sibling.Remove();
}


int Node::Query(Frustum& Fr, Vector<Node*>& Result, int Types, bool inside) {
	// test if we are the one
	if (Type & Types) {
		Node* node = this;
		Result.PushBack(node);
	}
	LinkList<Node>::Iterator Iter;
	for (Iter = SubNodes.Begin(); Iter != SubNodes.End(); Iter++) {
		Node* sub = *Iter;
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
			if (sub->Type & CLIPMAP) {
				// clipmap is always visible
				inside = false;
				break;
			}
			continue;
		default:
			continue;
		}
		sub->Query(Fr, Result, Types, inside);
	}
	return 0;
}

int Node::Query(Vector<Node*>& Result, int Types) {
	if (Type & Types) {
		Node* node = this;
		Result.PushBack(node);
	}
	// sub nodes
	for (auto Iter = SubNodes.Begin(); Iter != SubNodes.End(); Iter++) {
		auto sub = *Iter;
		sub->Query(Result, Types);
	}
	return 0;
}

int Node::Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context) {
	return 0;
}

/*
	update  raytracing infos need for raytracing. bottom level as, top level as. and shader tables
*/
int Node::UpdateRaytracingStructure(CommandBuffer* cmdBuffer, RenderingCamera* camera, RenderContext* renderContext) {
	return 0;
}

int Node::Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext)
{
	return 0;
}

void Node::SyncCullingObject() {
	Vector3 Center = CullingObj.LocalCenter;
	//Center = Center * Rotation;
	//Center = Center + Position;
	Center = Center * GetWorldMatrix();
	CullingObj.Translate(Center);
}


void Node::SetPosition(Vector3& Position_) {
	Position = Position_;
	SyncCullingObject();
}

void Node::SetRotation(Quaternion& Rot) {
	Rotation = Rot;
	SyncCullingObject();
}

Matrix4x4& Node::GetWorldMatrix() {
	Transform.Tranform(Position, Rotation);
	return Transform;
}
