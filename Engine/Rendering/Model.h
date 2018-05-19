#ifndef __MODEL__
#define __MODEL__


#include "Mesh.h"
//#include "RenderEngine.h"


/*
	model component for rendering system
*/

class Model: public Object {
public:
	// macros
	OBJECT(Model);
	BASEOBJECT(Model);
	DECLAR_ALLOCATER(Model);

private:
	// pointer for store user data
	void * UserData;

public:
	// mesh name for easy display
	char MeshName[64];
	// reference to mesh resource
	Mesh * MeshResource[1];
public:
	Model(Context * context);
	virtual ~Model();
	//set loaded mesh to model
	int SetMesh(Mesh * mesh, int Lod);
	// set name
	void SetName(char * Name);
	// set UserDara
	void SetUserData(void * pointer) { UserData = pointer; }
	// get UserData
	void * GetUserData() { return UserData; };
	// get Mesg
	Mesh * GetMesh(int Lod) { return MeshResource[Lod]; }
	// get name
	char * GetName() { return MeshName; };

};


#endif

