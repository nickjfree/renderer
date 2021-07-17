#include "Model.h"

USING_ALLOCATER(Model);

Model::Model(Context* context) : Object(context), UserData(0) {
}


Model::~Model() {
}

int Model::SetMesh(Mesh* mesh, int Lod) {
	MeshResource[Lod] = mesh;
	// test for event
	/*Event * Ev = Event::Create();
	Ev->EventId = 200;
	Ev->EventParam["mesh"] = mesh;
	BroadCast(Ev);
	Ev->Recycle();*/
	return 0;
}


void Model::SetName(char* Name) {
	strcpy_s(MeshName, Name);
}
