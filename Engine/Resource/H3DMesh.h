#ifndef __H3D_MESH__
#define __H3D_MESH__


#include "Mesh.h"
#include "RenderEngine.h"

/*
	H3D version mesh implementation,  bridge from CEntity to Mesh
*/
class H3DMesh : public Mesh
{
	DECLAR_ALLOCATER(H3DMesh);
	OBJECT(Mesh);
public: // for test, use public so meshrenderer can access the entity.  the final version should not be public
	// render engine entity reference, 
	CEntity * Entity;
	// render engine scenemanager interface
	CSceneManager * SceneManager;
public:
	H3DMesh(Context * context);
	h3d_mesh * GetH3DMesh(h3d_header * Header, int MeshIndex);
	virtual ~H3DMesh();
	virtual int OnSerialize(void * Header);
	virtual int OnLoadComplete(void * Data);
	virtual int OnCreateComplete(void * Data);
};


#endif
