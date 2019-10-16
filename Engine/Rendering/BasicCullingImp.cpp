#include "BasicCullingImp.h"



BasicCullingImp::BasicCullingImp() {
	if (!Root)
		Root = new Node();
}

BasicCullingImp::~BasicCullingImp()
{
	if (Root) {
		delete Root;
	}
}

int BasicCullingImp::Query(Frustum& Fr, Vector<Node*>& Result, int Type) {
	Root->Query(Fr, Result, Type, 0);
	return 0;
}

int BasicCullingImp::Query(AABB& Box, Vector<Node*>& Result, int Type) {
	return 0;
}

// insert
int BasicCullingImp::Add(RenderObject* RenderObj) {
	CullingObject& cul_obj = RenderObj->GetCullingObject();
	// use box for some other use
	Root->AddChild(RenderObj);
	return 0;
}
//remove
int BasicCullingImp::Remove(RenderObject* RenderObj) {
	Root->Remove(RenderObj);
	return 0;
}
