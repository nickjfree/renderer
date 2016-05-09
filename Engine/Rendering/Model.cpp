#include "Model.h"

USING_ALLOCATER(Model);

Model::Model()
{
}


Model::~Model()
{
}

int Model::SetMesh(Mesh * mesh, int Lod) {
	MeshResource[Lod] = mesh;
	// test for event
	/*Event * Ev = Event::Create();
	Ev->EventId = 200;
	Ev->EventParam["mesh"].as<Mesh*>() = mesh;
	BroadCast(Ev);
	Ev->Recycle();*/
	return 0;
}
