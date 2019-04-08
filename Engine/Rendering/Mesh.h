#ifndef __MESH__
#define __MESH__

#include "GPUResource.h"
#include "Math\Collision.h"
#include "Math\LinearMath.h"

/*
    the mesh resource
*/

typedef struct MeshConvex {
    int VNum;
    int INum;
    float * VBuffer;
    unsigned int * IBuffer;
    Vector3 Center;
}MeshConvex;



class Mesh : public GPUResource
{
    DECLAR_ALLOCATER(Mesh);
    OBJECT(Mesh);
protected:
    AABB Box;
public:
    // graphic data
    DWORD VBSize;
    DWORD VTSize;
    void * VBuffer;
    DWORD INum;
    WORD * IBuffer;
    // physics data
    int NumConvex;
    MeshConvex * ConvexHulls;
public:
    Mesh(Context* context);
    virtual ~Mesh();
    AABB& GetAABB() { return Box; };
};

#endif
