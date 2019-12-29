#include "Spatial.h"


Spatial::Spatial() : Root(0)
{
}


Spatial::~Spatial()
{
}

int Spatial::Add(RenderObject* RenderObj) {
	return 0;
}

int Spatial::Remove(RenderObject* RenderObj) {
	return 0;
}

int Spatial::Query(Frustum& Fr, Vector<Node*>& Result, int Type) {
	return 0;
}

int Spatial::Query(AABB& Box, Vector<Node*>& Result, int Type) {
	return 0;
}

int Spatial::Query(Vector<Node*>& Result, int Type) {
	return 0;
}
