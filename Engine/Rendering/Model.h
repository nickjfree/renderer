#ifndef __MODEL__
#define __MODEL__


#include "Mesh.h"
//#include "RenderEngine.h"


/*
	model component for rendering system
*/

class Model
{
public:
	// macros
	OBJECT(Model);
	BASEOBJECT(Model);
	DECLAR_ALLOCATER(Model);

public:
	// mesh name for easy display
	char * MeshName;
	// reference to mesh resource
	Mesh * MeshResource[1];
public:
	Model();
	virtual ~Model();
	//set loaded mesh to model
	int SetMesh(Mesh * mesh, int Lod);

};


#endif

