#ifndef __H3D_MESH__
#define __H3D_MESH__


#include "Mesh.h"
//#include "RenderEngine.h"
#include "RenderInterface.h"
#include "RenderingSystem.h"
#include "Misc\h3d\h3d.h"

/*
    H3D version mesh implementation.
*/

using namespace h3d;


class H3DMesh : public Mesh
{
    DECLAR_ALLOCATER(H3DMesh);
    OBJECT(H3DMesh);
    BASEOBJECT(Mesh);
    LOADEROBJECT(FileMappingLoader);

private:
    // compute convexhulls
    void ComputeConvexHull();
public:
    H3DMesh(Context * context);
    h3d_mesh * GetH3DMesh(h3d_header * Header, int MeshIndex);
    virtual ~H3DMesh();
    virtual int OnSerialize(Deserializer& deserializer);
    virtual int OnLoadComplete(Variant& Data);
    virtual int OnCreateComplete(Variant& Data);
};


#endif
